/**
 * @file flight_state.c
 * @brief Implementação da máquina de estados do foguete.
 */

#include "flight_state.h"
#include "fusion.h"
#include "flight_defs.h"
#include "hal_plataforma.h"
#include <stddef.h>

static flight_state_t estado_atual = FSM_BOOT;
static uint32_t tempo_inicio_estado_ms = 0;
static fsm_callback_transicao_t callback_transicao = NULL;

static const char* nomes_estados[] = {
    "BOOT",
    "IDLE",
    "ARMED",
    "ASCENT",
    "APOGEE",
    "DESCENT",
    "LANDED"
};

static void fsm_mudar_estado(flight_state_t novo_estado) {
    flight_state_t anterior = estado_atual;
    estado_atual = novo_estado;
    tempo_inicio_estado_ms = plataforma_obter_tick_ms();

    if (callback_transicao != NULL) {
        callback_transicao(anterior, novo_estado);
    }
}

status_t fsm_inicializar(void) {
    estado_atual = FSM_BOOT;
    tempo_inicio_estado_ms = plataforma_obter_tick_ms();

    /* Transição automática do boot para ocioso */
    fsm_mudar_estado(FSM_IDLE);

    return STATUS_OK;
}

status_t fsm_atualizar(const dados_sensores_t *dados) {
    if (!dados) return STATUS_ERRO_GENERICO;

    switch (estado_atual) {
        case FSM_IDLE:
            break;

        case FSM_ARMED:
            if (fusao_detectar_lancamento()) {
                fsm_mudar_estado(FSM_ASCENT);
            }
            /* Timeout se ficar armado por muito tempo sem lançar */
            else if (fsm_tempo_no_estado() > TEMPO_TIMEOUT_ARMADO_MS) {
                fsm_mudar_estado(FSM_IDLE);
            }
            break;

        case FSM_ASCENT:
            if (fusao_detectar_apogeu()) {
                fsm_mudar_estado(FSM_APOGEE);
            }
            break;

        case FSM_APOGEE:
            /* Normalmente a transição de APOGEE para DESCENT é rápida após acionamento */
            if (fsm_tempo_no_estado() > 2000) { /* 2 segundos após apogeu */
                fsm_mudar_estado(FSM_DESCENT);
            }
            break;

        case FSM_DESCENT:
            if (fusao_detectar_pouso()) {
                fsm_mudar_estado(FSM_LANDED);
            }
            break;

        case FSM_LANDED:
            break;

        default:
            break;
    }

    return STATUS_OK;
}

flight_state_t fsm_obter_estado(void) {
    return estado_atual;
}

status_t fsm_processar_comando(command_id_t comando) {
    if (!comando_valido_no_estado(comando, estado_atual)) {
        return STATUS_ERRO_GENERICO;
    }

    switch (comando) {
        case CMD_ARM:
            if (transicao_valida(estado_atual, FSM_ARMED)) {
                fsm_mudar_estado(FSM_ARMED);
            }
            break;

        case CMD_DISARM:
            if (transicao_valida(estado_atual, FSM_IDLE)) {
                fsm_mudar_estado(FSM_IDLE);
            }
            break;

        default:
            break;
    }

    return STATUS_OK;
}

const char* fsm_nome_estado(flight_state_t estado) {
    if (estado <= FSM_LANDED) {
        return nomes_estados[estado];
    }
    return "UNKNOWN";
}

uint32_t fsm_tempo_no_estado(void) {
    return plataforma_obter_tick_ms() - tempo_inicio_estado_ms;
}

void fsm_registrar_callback(fsm_callback_transicao_t callback) {
    callback_transicao = callback;
}
