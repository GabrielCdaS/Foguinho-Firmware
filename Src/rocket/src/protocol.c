/**
 * @file protocol.c
 * @brief Implementação do módulo de recepção e processamento de comandos.
 */

#include "protocol.h"
#include "sx1278.h"
#include "crc16.h"
#include <stddef.h>

#define MAX_COMANDOS_FILA 10

static command_id_t fila_comandos[MAX_COMANDOS_FILA];
static uint8_t fila_inicio = 0;
static uint8_t fila_fim = 0;
static uint8_t fila_tamanho = 0;

static protocolo_callback_comando_t callback_comando = NULL;

status_t protocolo_inicializar(void) {
    fila_inicio = 0;
    fila_fim = 0;
    fila_tamanho = 0;
    callback_comando = NULL;
    return STATUS_OK;
}

status_t protocolo_processar_recebidos(void) {
    uint8_t buffer[64];
    uint8_t tamanho = 0;

    status_t res = sx1278_receber(buffer, &tamanho, 0);
    if (res == STATUS_OK && tamanho == sizeof(command_packet_t)) {
        command_packet_t *pacote = (command_packet_t *)buffer;

        if (pacote->header == PROTOCOL_HEADER_BYTE) {
            uint16_t crc_calculado = crc16_calculate(buffer, tamanho - 2);
            if (crc_calculado == pacote->crc16) {
                /* CRC Válido, enfileirar comando */
                if (fila_tamanho < MAX_COMANDOS_FILA) {
                    fila_comandos[fila_fim] = (command_id_t)pacote->command_id;
                    fila_fim = (fila_fim + 1) % MAX_COMANDOS_FILA;
                    fila_tamanho++;

                    if (callback_comando != NULL) {
                        callback_comando((command_id_t)pacote->command_id);
                    }
                }
            }
        }
    }

    return STATUS_OK;
}

bool protocolo_comando_disponivel(void) {
    return (fila_tamanho > 0);
}

status_t protocolo_obter_comando(command_id_t *comando) {
    if (fila_tamanho == 0) {
        return STATUS_ERRO_GENERICO;
    }

    if (comando) {
        *comando = fila_comandos[fila_inicio];
    }

    fila_inicio = (fila_inicio + 1) % MAX_COMANDOS_FILA;
    fila_tamanho--;

    return STATUS_OK;
}

status_t protocolo_enviar_resposta(command_id_t comando, uint8_t resultado) {
    command_packet_t resposta;
    resposta.header = PROTOCOL_HEADER_BYTE;
    resposta.command_id = comando;
    resposta.payload = resultado;

    resposta.crc16 = crc16_calculate((uint8_t*)&resposta, sizeof(command_packet_t) - 2);

    return sx1278_transmitir((uint8_t*)&resposta, sizeof(command_packet_t));
}

void protocolo_registrar_callback(protocolo_callback_comando_t callback) {
    callback_comando = callback;
}
