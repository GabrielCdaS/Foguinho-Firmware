/**
 * @file command.h
 * @brief Módulo de envio de comandos da estação base para o foguete.
 *
 * Gerencia o envio de comandos do PC (via USB) para o foguete
 * (via LoRa). Suporta os comandos definidos no protocolo comum:
 *   - ARM (0x01): armar o foguete
 *   - DISARM (0x02): desarmar o foguete
 *   - PING (0x03): verificar conectividade
 *   - START_LOG (0x04): iniciar gravação de dados
 *   - STOP_LOG (0x05): parar gravação de dados
 *
 * O módulo também controla o mecanismo de confirmação de resposta
 * (acknowledge), permitindo verificar se o foguete recebeu e
 * processou o comando enviado.
 */

#ifndef COMMAND_H
#define COMMAND_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================== */
/*                    Inclusões de Dependências                   */
/* ============================================================== */

#include <stdint.h>
#include <stdbool.h>
#include "protocol.h"
#include "../../common/inc/protocol_defs.h"

/* ============================================================== */
/*                   Funções de Inicialização                     */
/* ============================================================== */

/**
 * @brief Inicializa o módulo de comandos.
 *
 * Reseta o estado interno, zera contadores e prepara o módulo
 * para enviar comandos ao foguete via rádio LoRa.
 *
 * @return STATUS_OK em caso de sucesso.
 */
status_t comando_inicializar(void);

/* ============================================================== */
/*                Funções de Envio de Comandos                    */
/* ============================================================== */

/**
 * @brief Envia o comando ARM (0x01) para o foguete.
 *
 * Solicita que o foguete entre no estado ARMED, habilitando
 * a detecção de lançamento e acionamento do paraquedas.
 *
 * @return STATUS_OK se o comando foi transmitido com sucesso.
 * @return STATUS_ERRO_RADIO se houve falha na transmissão LoRa.
 */
status_t comando_enviar_arm(void);

/**
 * @brief Envia o comando DISARM (0x02) para o foguete.
 *
 * Solicita que o foguete retorne ao estado IDLE, desabilitando
 * os mecanismos de segurança e acionamento.
 *
 * @return STATUS_OK se o comando foi transmitido com sucesso.
 * @return STATUS_ERRO_RADIO se houve falha na transmissão LoRa.
 */
status_t comando_enviar_disarm(void);

/**
 * @brief Envia o comando PING (0x03) para o foguete.
 *
 * Verifica a conectividade com o foguete. Espera-se que o
 * foguete responda com um pacote de confirmação.
 *
 * @return STATUS_OK se o comando foi transmitido com sucesso.
 * @return STATUS_ERRO_RADIO se houve falha na transmissão LoRa.
 */
status_t comando_enviar_ping(void);

/**
 * @brief Envia o comando START_LOG (0x04) para o foguete.
 *
 * Solicita que o foguete inicie a gravação de dados de
 * telemetria no cartão SD interno.
 *
 * @return STATUS_OK se o comando foi transmitido com sucesso.
 * @return STATUS_ERRO_RADIO se houve falha na transmissão LoRa.
 */
status_t comando_enviar_iniciar_log(void);

/**
 * @brief Envia o comando STOP_LOG (0x05) para o foguete.
 *
 * Solicita que o foguete pare a gravação de dados de
 * telemetria no cartão SD interno.
 *
 * @return STATUS_OK se o comando foi transmitido com sucesso.
 * @return STATUS_ERRO_RADIO se houve falha na transmissão LoRa.
 */
status_t comando_enviar_parar_log(void);

/**
 * @brief Envia um comando genérico para o foguete.
 *
 * Permite enviar qualquer comando válido definido no protocolo,
 * identificado pelo seu command_id_t.
 *
 * @param[in] cmd  Identificador do comando a enviar.
 *
 * @return STATUS_OK se o comando foi transmitido com sucesso.
 * @return STATUS_ERRO_RADIO se houve falha na transmissão LoRa.
 */
status_t comando_enviar_generico(command_id_t cmd);

/* ============================================================== */
/*              Funções de Controle de Resposta                   */
/* ============================================================== */

/**
 * @brief Verifica se há um comando aguardando resposta do foguete.
 *
 * Indica se o último comando enviado ainda não recebeu
 * confirmação (acknowledge) do foguete.
 *
 * @return true se aguardando resposta, false caso contrário.
 */
bool comando_aguardando_resposta(void);

/**
 * @brief Aguarda e verifica a resposta do foguete ao último comando.
 *
 * Bloqueia (polling) até receber uma resposta do foguete ou
 * até o timeout expirar. Verifica se a resposta corresponde
 * ao comando enviado.
 *
 * @param[in] timeout_ms  Tempo máximo de espera em milissegundos.
 *
 * @return STATUS_OK se a resposta foi recebida e validada.
 * @return STATUS_ERRO_TIMEOUT se o timeout expirou sem resposta.
 * @return STATUS_ERRO_RADIO se houve erro na recepção via rádio.
 */
status_t comando_verificar_resposta(uint32_t timeout_ms);

/* ============================================================== */
/*                   Funções de Estatísticas                      */
/* ============================================================== */

/**
 * @brief Retorna o número total de comandos enviados.
 *
 * Contabiliza todos os comandos transmitidos com sucesso
 * pelo módulo de rádio desde a inicialização.
 *
 * @return Contagem de comandos enviados.
 */
uint32_t comando_comandos_enviados(void);

#ifdef __cplusplus
}
#endif

#endif /* COMMAND_H */
