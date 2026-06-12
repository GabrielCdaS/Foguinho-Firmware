/**
 * @file flight_defs.c
 * @brief Implementação das validações de estado e comandos.
 */

#include "flight_defs.h"

bool transicao_valida(flight_state_t atual, flight_state_t proxima) {
    switch (atual) {
        case FSM_BOOT:
            return (proxima == FSM_IDLE);
        case FSM_IDLE:
            return (proxima == FSM_ARMED);
        case FSM_ARMED:
            return (proxima == FSM_ASCENT || proxima == FSM_IDLE);
        case FSM_ASCENT:
            return (proxima == FSM_APOGEE || proxima == FSM_DESCENT);
        case FSM_APOGEE:
            return (proxima == FSM_DESCENT);
        case FSM_DESCENT:
            return (proxima == FSM_LANDED);
        case FSM_LANDED:
            return false; /* Estado terminal */
        default:
            return false;
    }
}

bool comando_valido_no_estado(command_id_t cmd, flight_state_t estado) {
    switch (cmd) {
        case CMD_ARM:
            return (estado == FSM_IDLE);
        case CMD_DISARM:
            return (estado == FSM_ARMED);
        case CMD_PING:
            return true; /* PING válido em qualquer estado */
        case CMD_START_LOG:
        case CMD_STOP_LOG:
            /* Logs podem ser forçados nestes estados */
            return (estado == FSM_IDLE || estado == FSM_ARMED || estado == FSM_LANDED);
        default:
            return false;
    }
}
