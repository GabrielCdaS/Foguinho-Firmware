/**
 * @file protocol.c
 * @brief Implementação do módulo de recepção e processamento de comandos.
 */

#include "protocol.h"
#include "sx1278.h"
#include "crc16.h"
#include <stddef.h>
#include <string.h>

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
    uint8_t buffer[SX1278_TAMANHO_MAX_PACOTE];
    uint8_t tamanho = 0;

    status_t res = sx1278_receber(buffer, &tamanho, 0);
    if (res == STATUS_ERRO_TIMEOUT) return STATUS_OK;
    if (res != STATUS_OK) return res;
    if (tamanho != sizeof(command_packet_t)) return STATUS_ERRO_GENERICO;

    command_packet_t pacote;
    memcpy(&pacote, buffer, sizeof(pacote));
    if (pacote.header != PROTOCOL_HEADER_BYTE) return STATUS_ERRO_GENERICO;
    if (!crc16_verificar(buffer, (uint16_t)(tamanho - sizeof(pacote.crc16)), pacote.crc16)) {
        return STATUS_ERRO_CRC;
    }
    if (pacote.command_id < CMD_ARM || pacote.command_id > CMD_STOP_LOG) {
        return STATUS_ERRO_GENERICO;
    }
    if (fila_tamanho >= MAX_COMANDOS_FILA) return STATUS_ERRO_GENERICO;

    command_id_t comando = (command_id_t)pacote.command_id;
    fila_comandos[fila_fim] = comando;
    fila_fim = (uint8_t)((fila_fim + 1U) % MAX_COMANDOS_FILA);
    fila_tamanho++;
    if (callback_comando != NULL) callback_comando(comando);
    return STATUS_OK;
}

bool protocolo_comando_disponivel(void) {
    return (fila_tamanho > 0);
}

status_t protocolo_obter_comando(command_id_t *comando) {
    if (!comando || fila_tamanho == 0U) {
        return STATUS_ERRO_GENERICO;
    }

    *comando = fila_comandos[fila_inicio];

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
