#include "command_executor.h"

#include "config.h"
#include "datalogger.h"
#include "flight_defs.h"
#include "flight_state.h"
#include "recovery.h"
#include "sensors.h"

static command_result_t resultado_de_status(status_t status) {
    if (status == STATUS_OK) return COMMAND_RESULT_OK;
    if (status == STATUS_ERRO_SD) return COMMAND_RESULT_STORAGE_ERROR;
    return COMMAND_RESULT_HARDWARE_ERROR;
}

command_result_t comando_executar(command_id_t comando) {
    flight_state_t estado = fsm_obter_estado();
    if (!comando_valido_no_estado(comando, estado)) {
        return COMMAND_RESULT_INVALID_STATE;
    }

    switch (comando) {
        case CMD_ARM: {
            status_t status = recuperacao_armar();
            if (status != STATUS_OK) return COMMAND_RESULT_SAFETY_LOCK;
            status = fsm_processar_comando(comando);
            if (status != STATUS_OK) recuperacao_desarmar();
            return status == STATUS_OK ? COMMAND_RESULT_OK : COMMAND_RESULT_INVALID_STATE;
        }

        case CMD_DISARM:
            return fsm_processar_comando(comando) == STATUS_OK
                ? COMMAND_RESULT_OK
                : COMMAND_RESULT_INVALID_STATE;

        case CMD_PING:
            return COMMAND_RESULT_OK;

        case CMD_START_LOG:
            #if HABILITAR_DATALOGGER
            return resultado_de_status(
                datalogger_abrir_arquivo(DATALOGGER_NOME_ARQUIVO_PADRAO)
            );
            #else
            return COMMAND_RESULT_STORAGE_ERROR;
            #endif

        case CMD_STOP_LOG:
            #if HABILITAR_DATALOGGER
            return datalogger_esta_pronto()
                ? resultado_de_status(datalogger_fechar())
                : COMMAND_RESULT_STORAGE_ERROR;
            #else
            return COMMAND_RESULT_STORAGE_ERROR;
            #endif

        default:
            return COMMAND_RESULT_INVALID_STATE;
    }
}
