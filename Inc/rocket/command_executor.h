#ifndef COMMAND_EXECUTOR_H
#define COMMAND_EXECUTOR_H

#include "protocol_defs.h"

/**
 * @brief Executa um comando recebido e retorna um resultado compartilhado.
 */
command_result_t comando_executar(command_id_t comando);

#endif
