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
#include "command_executor.h"
#include "gps.h"
#include "protocol.h"
#include "recovery.h"
#include "sx1278.h"
#include "hal_plataforma.h"
#include "stm32f411_hw.h"

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
    #if HABILITAR_RECUPERACAO
    recuperacao_processar();
    #endif
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
            /* Inicia o registro automaticamente ao armar, quando disponível. */
            #if HABILITAR_DATALOGGER
            if (!datalogger_esta_pronto()) {
                datalogger_abrir_arquivo(DATALOGGER_NOME_ARQUIVO_PADRAO);
            }
            #endif
            break;

        case FSM_IDLE:
            recuperacao_desarmar();
            break;

        case FSM_APOGEE:
            /* O canal de emergência só atua se o acionamento principal falhar. */
            #if HABILITAR_RECUPERACAO
            if (recuperacao_acionar_principal() != STATUS_OK) {
                recuperacao_acionar_emergencia();
            }
            #endif
            break;

        case FSM_DESCENT:
            break;

        case FSM_LANDED:
            /* Foguete pousou com segurança */
            #if HABILITAR_DATALOGGER
            if (datalogger_esta_pronto()) {
                datalogger_flush();
                datalogger_fechar();
            }
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

    #if HABILITAR_TELEMETRIA
    sx1278_inicializar();
    #endif

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
    dados_sensores_t sensores = {0};
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

        #if HABILITAR_GPS
        gps_processar();
        #endif

        /* Comunicação e comandos continuam ativos mesmo se um sensor falhar. */
        #if HABILITAR_TELEMETRIA
        sx1278_processar();
        protocolo_processar_recebidos();
        command_id_t cmd;
        while (protocolo_comando_disponivel()) {
            if (protocolo_obter_comando(&cmd) == STATUS_OK) {
                command_result_t resultado = comando_executar(cmd);
                protocolo_enviar_resposta(cmd, (uint8_t)resultado);
            }
        }
        #endif

        /* Lê todos os sensores da IMU e Barômetro */
        bool amostra_valida = sensores_ler_todos(&sensores) == STATUS_OK;

        /* Atualiza a fusão sensorial (Altitude, Velocidade) */
        if (amostra_valida) {
            amostra_valida = fusao_atualizar(&sensores) == STATUS_OK;
        }

        /* Atualiza a máquina de estados (Detecta lançamento, apogeu, etc) */
        if (amostra_valida) {
            fsm_atualizar(&sensores);
        }

        /* Grava dados no cartão SD */
        #if HABILITAR_DATALOGGER
        if (amostra_valida && datalogger_esta_pronto() && fsm_obter_estado() > FSM_IDLE) {
            datalogger_escrever_sensores(&sensores);
        }
        #endif

        /* ------------------------------------------------------------------
         * Tarefas de 10 Hz (Executadas a cada 20 ticks)
         * ------------------------------------------------------------------ */
        if (amostra_valida &&
            (contador_ticks % (FREQ_AMOSTRAGEM_SENSORES_HZ / FREQ_TELEMETRIA_HZ)) == 0) {
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
        }
    }

    return 0;
}
