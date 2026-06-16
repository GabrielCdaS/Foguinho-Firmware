/**
 * @file telemetry.h
 * @brief Módulo de telemetria — construção e envio de pacotes.
 *
 * Constrói pacotes de telemetria a partir dos dados dos sensores e do
 * estado de voo, e os transmite via rádio LoRa (SX1278) a 10 Hz.
 *
 * Os tipos telemetry_packet_t e flight_state_t são definidos no cabeçalho
 * comum protocol_defs.h, compartilhado entre o foguete e a estação base.
 */

#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdint.h>
#include "protocol_defs.h"
#include "sensors.h"

/* ============================================================================
 * Funções do Módulo de Telemetria
 * ========================================================================= */

/**
 * @brief Inicializa o módulo de telemetria.
 *
 * Zera contadores de pacotes e configura o estado interno do módulo.
 * O rádio SX1278 deve ser inicializado separadamente.
 *
 * @return STATUS_OK em caso de sucesso.
 */
status_t telemetria_inicializar(void);

/**
 * @brief Constrói um pacote de telemetria a partir dos dados dos sensores.
 *
 * Preenche a estrutura do pacote com os dados de sensores, estado de voo,
 * ID sequencial do pacote e calcula o CRC.
 *
 * @param[out] pacote Ponteiro para a estrutura do pacote a ser preenchida.
 * @param[in]  dados  Dados mais recentes dos sensores.
 * @param[in]  estado Estado de voo atual.
 * @return STATUS_OK em caso de sucesso.
 */
status_t telemetria_construir_pacote(telemetry_packet_t *pacote,
                                     const dados_sensores_t *dados,
                                     flight_state_t estado);

/**
 * @brief Envia o pacote de telemetria mais recente via rádio LoRa.
 *
 * Constrói e transmite automaticamente um pacote de telemetria.
 * Incrementa o contador de pacotes enviados e atualiza o timestamp
 * do último envio.
 *
 * @return STATUS_OK em caso de sucesso, STATUS_ERRO_RADIO em caso de falha.
 */
status_t telemetria_enviar(void);

/**
 * @brief Obtém o ID sequencial do próximo pacote a ser enviado.
 * @return ID do pacote (0-255, com wrap-around).
 */
uint8_t telemetria_obter_id_pacote(void);

/**
 * @brief Retorna o número total de pacotes de telemetria enviados.
 * @return Contador de pacotes enviados desde a inicialização.
 */
uint32_t telemetria_pacotes_enviados(void);

/**
 * @brief Retorna o timestamp do último envio de telemetria.
 * @return Timestamp em milissegundos do último pacote enviado.
 */
uint32_t telemetria_ultimo_envio_ms(void);

#endif /* TELEMETRY_H */
