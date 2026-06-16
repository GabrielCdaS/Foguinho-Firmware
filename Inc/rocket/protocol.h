/**
 * @file protocol.h
 * @brief Módulo de recepção e processamento de comandos.
 *
 * Gerencia a recepção de comandos enviados pela estação base via rádio LoRa,
 * processa a fila de comandos recebidos e envia respostas de confirmação.
 *
 * O tipo command_id_t é definido no cabeçalho comum protocol_defs.h,
 * compartilhado entre o foguete e a estação base.
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>
#include "protocol_defs.h"
#include "sensors.h"

/* ============================================================================
 * Tipos de Callback
 * ========================================================================= */

/**
 * @brief Tipo de callback chamado quando um comando válido é recebido.
 *
 * Permite que módulos externos sejam notificados imediatamente quando
 * um novo comando é processado com sucesso.
 *
 * @param comando Identificador do comando recebido.
 */
typedef void (*protocolo_callback_comando_t)(command_id_t comando);

/* ============================================================================
 * Funções do Módulo de Protocolo
 * ========================================================================= */

/**
 * @brief Inicializa o módulo de protocolo de comunicação.
 *
 * Configura os buffers internos e prepara o sistema para recepção
 * de comandos via rádio.
 *
 * @return STATUS_OK em caso de sucesso.
 */
status_t protocolo_inicializar(void);

/**
 * @brief Processa os dados recebidos pelo rádio.
 *
 * Verifica se há dados disponíveis no buffer de recepção, valida o
 * formato do pacote, verifica o CRC e extrai os comandos.
 *
 * Deve ser chamada periodicamente no loop principal.
 *
 * @return STATUS_OK se processou com sucesso (mesmo sem dados novos).
 */
status_t protocolo_processar_recebidos(void);

/**
 * @brief Verifica se há algum comando disponível na fila.
 * @return true se há pelo menos um comando aguardando processamento.
 */
bool protocolo_comando_disponivel(void);

/**
 * @brief Retira o próximo comando da fila de comandos recebidos.
 *
 * Remove e retorna o comando mais antigo da fila FIFO.
 *
 * @param[out] comando Ponteiro para receber o identificador do comando.
 * @return STATUS_OK se um comando foi obtido, STATUS_ERRO_GENERICO se
 *         a fila está vazia.
 */
status_t protocolo_obter_comando(command_id_t *comando);

/**
 * @brief Envia uma resposta/confirmação para a estação base.
 *
 * Constrói um pacote de resposta contendo o ID do comando e o resultado,
 * calcula o CRC e transmite via rádio LoRa.
 *
 * @param[in] comando   Identificador do comando que está sendo respondido.
 * @param[in] resultado Código de resultado (0 = sucesso, outros = erro).
 * @return STATUS_OK se a resposta foi enviada com sucesso.
 */
status_t protocolo_enviar_resposta(command_id_t comando, uint8_t resultado);

/**
 * @brief Registra uma função de callback para recepção de comandos.
 *
 * O callback será chamado automaticamente quando um novo comando válido
 * for recebido e validado.
 *
 * @param[in] callback Ponteiro para a função de callback.
 */
void protocolo_registrar_callback(protocolo_callback_comando_t callback);

#endif /* PROTOCOL_H */
