/**
 * @file flight_defs.h
 * @brief Definições de eventos de voo e limiares para detecção de fases.
 *
 * Contém os limiares utilizados pela FSM de voo para identificar
 * transições entre estados (lançamento, apogeu, descida, pouso),
 * além de funções de validação de transições e comandos.
 *
 * Compartilhado entre o firmware do foguete e da estação base.
 */

#ifndef FLIGHT_DEFS_H
#define FLIGHT_DEFS_H

#include <stdbool.h>
#include "protocol_defs.h"

/* ============================================================
 * Limiares de detecção de eventos de voo
 * ============================================================ */

/**
 * Limiar de aceleração para detectar o lançamento (em g).
 * Quando a aceleração excede este valor, a FSM transita de
 * ARMED para ASCENT.
 */
#define LIMIAR_ACELERACAO_LANCAMENTO_G    (3.0f)

/**
 * Limiar de velocidade vertical para detecção de apogeu (em m/s).
 * Quando a velocidade vertical cai abaixo deste valor (próxima de zero),
 * indica que o foguete atingiu o ponto mais alto da trajetória.
 */
#define LIMIAR_VELOCIDADE_APOGEU_MS      (1.0f)

/**
 * Limiar de variação de altitude para confirmação de pouso (em metros).
 * Se a altitude variar menos que este valor durante o período de
 * confirmação, considera-se que o foguete pousou.
 */
#define LIMIAR_VARIACAO_ALTITUDE_POUSO_M (2.0f)

/**
 * Limiar de aceleração para confirmação de pouso (em g).
 * Aceleração próxima de 1g (gravidade) indica que o foguete
 * está em repouso na superfície.
 */
#define LIMIAR_ACELERACAO_POUSO_G        (1.2f)

/**
 * Tempo necessário para confirmar pouso (em milissegundos).
 * O foguete deve permanecer estável por este período antes
 * de transitar para o estado LANDED.
 */
#define TEMPO_CONFIRMACAO_POUSO_MS       (5000)

/**
 * Timeout do estado armado (em milissegundos).
 * Se o foguete permanecer armado por mais de 10 minutos
 * sem detectar lançamento, retorna automaticamente para IDLE.
 */
#define TEMPO_TIMEOUT_ARMADO_MS          (600000)

/* ============================================================
 * Protótipos de funções de validação
 * ============================================================ */

/**
 * @brief Verifica se uma transição de estado é válida.
 *
 * Consulta a tabela de transições permitidas da FSM de voo
 * para determinar se a mudança do estado atual para o próximo
 * estado é válida.
 *
 * @param atual   Estado atual da FSM.
 * @param proxima Estado de destino desejado.
 * @return        true se a transição for permitida,
 *                false caso contrário.
 */
bool transicao_valida(flight_state_t atual, flight_state_t proxima);

/**
 * @brief Verifica se um comando é válido no estado atual.
 *
 * Determina se o comando recebido da estação base pode ser
 * executado considerando o estado atual da máquina de estados.
 * Por exemplo, CMD_ARM só é válido no estado IDLE.
 *
 * @param cmd    Identificador do comando recebido.
 * @param estado Estado atual da FSM.
 * @return       true se o comando for válido no estado informado,
 *               false caso contrário.
 */
bool comando_valido_no_estado(command_id_t cmd, flight_state_t estado);

#endif /* FLIGHT_DEFS_H */
