/**
 * @file main.c
 * @brief Loop principal do firmware do foguete (STM32F411).
 *
 * Implementa a máquina de estados, leitura de sensores, datalogger, telemetria
 * e acionamento de recuperação em um ciclo determinístico de 200 Hz.
 */

#include <stdint.h>
#include <stdbool.h>

#include "config.h"
#include "sensors.h"
#include "fusion.h"
#include "flight_state.h"
#include "telemetry.h"
#include "datalogger.h"
#include "battery.h"
#include "gps.h"
#include "protocol.h"
#include "recovery.h"
#include "sx1278.h"
#include "hal_plataforma.h"

/* ============================================================================
 * Variáveis Globais de Sincronização
 * ========================================================================= */
volatile uint32_t ms_ticks = 0;
volatile bool tick_pendente = false;

/* ============================================================================
 * Tratador de Interrupção do SysTick
 * ========================================================================= */
void SysTick_Handler(void) {
    ms_ticks++;
    /* Dispara o tick da malha de controle a cada PERIODO_AMOSTRAGEM_SENSORES_MS (ex: 5 ms = 200 Hz) */
    if ((ms_ticks % PERIODO_AMOSTRAGEM_SENSORES_MS) == 0) {
        tick_pendente = true;
    }
}

/* ============================================================================
 * Callback de Transição de Estado
 * ========================================================================= */
static void callback_transicao_estado(flight_state_t anterior, flight_state_t novo) {
    /* Ignora se o estado não mudou */
    if (anterior == novo) return;

    /* Ações automáticas com base na transição */
    switch (novo) {
        case FSM_ARMED:
            /* Arma os paraquedas e abre arquivo de log */
            recuperacao_armar();
            #if HABILITAR_DATALOGGER
            datalogger_abrir_arquivo(DATALOGGER_NOME_ARQUIVO_PADRAO);
            #endif
            break;

        case FSM_APOGEE:
            /* Atingiu o apogeu: aciona o canal principal (drogue) */
            #if HABILITAR_RECUPERACAO
            recuperacao_acionar_principal();
            #endif
            break;

        case FSM_DESCENT:
            /* Início da descida principal (ou atingiu altitude do backup): aciona emergência */
            #if HABILITAR_RECUPERACAO
            /* Aqui a lógica pode ser acionar o backup apenas em uma altitude menor,
               mas para simplificar o FSM, se entrou em DESCENT pode-se acionar o main. */
            recuperacao_acionar_emergencia();
            #endif
            break;

        case FSM_LANDED:
            /* Foguete pousou com segurança */
            #if HABILITAR_DATALOGGER
            datalogger_flush();
            datalogger_fechar();
            #endif
            break;

        default:
            break;
    }
}

/* ============================================================================
 * Inicialização do Sistema
 * ========================================================================= */
static void sistema_inicializar(void) {
    /* 1. Inicializa o clock e SysTick */
    plataforma_inicializar_systick();

    /* 2. Inicializa os módulos de Hardware */
    #if HABILITAR_RECUPERACAO
    recuperacao_inicializar();
    #endif

    sensores_inicializar();

    #if HABILITAR_GPS
    gps_inicializar();
    #endif

    #if HABILITAR_DATALOGGER
    datalogger_inicializar();
    #endif

    bateria_inicializar();

    /* Nota: sx1278_inicializar pode depender da implementação real do rádio.
     * Como sx1278.h foi analisado, a assinatura normalmente ficaria disponível
     * em um módulo de inicialização mais alto, ou chamamos direto se tiver a API. */
    /* sx1278_inicializar(); */

    /* 3. Inicializa os módulos Lógicos */
    fusao_inicializar();
    fsm_inicializar();

    #if HABILITAR_TELEMETRIA
    telemetria_inicializar();
    protocolo_inicializar();
    #endif

    /* 4. Registra os callbacks */
    fsm_registrar_callback(callback_transicao_estado);

    /* 5. Calibração inicial (no solo) */
    fusao_calibrar();
}

/* ============================================================================
 * Função Principal (Main Loop)
 * ========================================================================= */
int main(void) {
    /* Inicializa toda a plataforma e sensores */
    sistema_inicializar();

    /* Variáveis de controle de fluxo e escalonamento */
    dados_sensores_t sensores;
    uint32_t contador_ticks = 0;

    /* Loop Infinito a 200 Hz */
    while (1) {
        /* Aguarda o próximo tick de 5 ms */
        if (!tick_pendente) {
            continue;
        }
        tick_pendente = false;
        contador_ticks++;

        /* ------------------------------------------------------------------
         * Tarefas de 200 Hz (Executadas a cada tick)
         * ------------------------------------------------------------------ */

        /* Lê todos os sensores da IMU e Barômetro */
        sensores_ler_todos(&sensores);

        /* Atualiza a fusão sensorial (Altitude, Velocidade) */
        fusao_atualizar(&sensores);

        /* Atualiza a máquina de estados (Detecta lançamento, apogeu, etc) */
        fsm_atualizar(&sensores);

        /* Processa comandos de rádio recebidos da base */
        #if HABILITAR_TELEMETRIA
        protocolo_processar_recebidos();
        command_id_t cmd;
        while (protocolo_comando_disponivel()) {
            if (protocolo_obter_comando(&cmd) == STATUS_OK) {
                fsm_processar_comando(cmd);
            }
        }
        #endif

        /* Grava dados no cartão SD */
        #if HABILITAR_DATALOGGER
        if (datalogger_esta_pronto() && fsm_obter_estado() > FSM_IDLE) {
            datalogger_escrever_sensores(&sensores);
        }
        #endif

        /* ------------------------------------------------------------------
         * Tarefas de 10 Hz (Executadas a cada 20 ticks)
         * ------------------------------------------------------------------ */
        if ((contador_ticks % (FREQ_AMOSTRAGEM_SENSORES_HZ / FREQ_TELEMETRIA_HZ)) == 0) {
            #if HABILITAR_TELEMETRIA
            /* Constrói e envia pacote de telemetria */
            telemetry_packet_t pacote;
            telemetria_construir_pacote(&pacote, &sensores, fsm_obter_estado());
            telemetria_enviar();
            #endif
        }

        /* ------------------------------------------------------------------
         * Tarefas de 2 Hz (Executadas a cada 100 ticks)
         * ------------------------------------------------------------------ */
        if ((contador_ticks % (FREQ_AMOSTRAGEM_SENSORES_HZ / 2)) == 0) {
            #if HABILITAR_DATALOGGER
            if (datalogger_esta_pronto()) {
                datalogger_flush();
            }
            #endif
        }

        /* ------------------------------------------------------------------
         * Tarefas de 1 Hz (Executadas a cada 200 ticks)
         * ------------------------------------------------------------------ */
        if ((contador_ticks % FREQ_AMOSTRAGEM_SENSORES_HZ) == 0) {
            bateria_ler_tensao_mv();

            #if HABILITAR_GPS
            gps_processar();
            #endif
        }
    }

    return 0;
}
