/**
 * @file flight_state.h
 * @brief Máquina de estados de voo do foguete.
 *
 * Gerencia as transições entre os estados de voo:
 * BOOT(0) → IDLE(1) → ARMED(2) → ASCENT(3) → APOGEE(4) → DESCENT(5) → LANDED(6)
 *
 * Os tipos flight_state_t e command_id_t são definidos no cabeçalho comum
 * protocol_defs.h, compartilhado entre o foguete e a estação base.
 */

#ifndef FLIGHT_STATE_H
#define FLIGHT_STATE_H

#include <stdint.h>
#include "../../common/inc/protocol_defs.h"
#include "sensors.h"

/* ============================================================================
 * Tipos de Callback
 * ========================================================================= */

/**
 * @brief Tipo de callback chamado em cada transição de estado.
 *
 * Permite que outros módulos (telemetria, datalogger, recuperação) sejam
 * notificados quando o estado de voo muda.
 *
 * @param anterior Estado de voo anterior à transição.
 * @param novo     Novo estado de voo após a transição.
 */
typedef void (*fsm_callback_transicao_t)(flight_state_t anterior, flight_state_t novo);

/* ============================================================================
 * Funções da Máquina de Estados de Voo
 * ========================================================================= */

/**
 * @brief Inicializa a máquina de estados no estado BOOT.
 *
 * Configura o estado inicial e zera contadores internos.
 *
 * @return STATUS_OK em caso de sucesso.
 */
status_t fsm_inicializar(void);

/**
 * @brief Atualiza a máquina de estados com dados de sensores.
 *
 * Avalia as condições de transição com base nos dados de fusão sensorial
 * e realiza transições automáticas quando apropriado (ex: detecção de
 * lançamento, apogeu, pouso).
 *
 * @param[in] dados Dados mais recentes dos sensores.
 * @return STATUS_OK em caso de sucesso.
 */
status_t fsm_atualizar(const dados_sensores_t *dados);

/**
 * @brief Obtém o estado de voo atual.
 * @return Estado de voo atual (flight_state_t).
 */
flight_state_t fsm_obter_estado(void);

/**
 * @brief Processa um comando recebido via rádio.
 *
 * Comandos podem forçar transições de estado (ex: ARM, DISARM)
 * ou executar ações específicas.
 *
 * @param[in] comando Identificador do comando a processar.
 * @return STATUS_OK se o comando foi processado com sucesso.
 */
status_t fsm_processar_comando(command_id_t comando);

/**
 * @brief Retorna o nome do estado de voo como string legível.
 *
 * Útil para debug e log. Exemplo: "BOOT", "IDLE", "ARMED", etc.
 *
 * @param[in] estado Estado de voo.
 * @return Ponteiro para string constante com o nome do estado.
 */
const char* fsm_nome_estado(flight_state_t estado);

/**
 * @brief Retorna o tempo decorrido no estado atual em milissegundos.
 *
 * Útil para timeout de estados e lógica de transição temporizada.
 *
 * @return Tempo no estado atual em milissegundos.
 */
uint32_t fsm_tempo_no_estado(void);

/**
 * @brief Registra uma função de callback para transições de estado.
 *
 * O callback será chamado sempre que ocorrer uma transição entre estados,
 * permitindo que módulos externos reajam (ex: iniciar gravação, acionar
 * recuperação).
 *
 * @param[in] callback Ponteiro para a função de callback.
 */
void fsm_registrar_callback(fsm_callback_transicao_t callback);

#endif /* FLIGHT_STATE_H */
