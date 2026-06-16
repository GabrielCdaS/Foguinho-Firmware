/**
 * @file telemetry_parser.c
 * @brief Parser incremental dos pacotes de telemetria da estação base.
 */

#include "telemetry_parser.h"

#include "crc16.h"

#include <stddef.h>
#include <string.h>

static parser_estado_t estado;
static uint8_t buffer[PROTOCOL_PACKET_SIZE];
static uint16_t posicao;
static telemetry_packet_t ultimo_pacote;
static bool pacote_disponivel;
static uint32_t pacotes_validos;
static uint32_t erros_crc;

status_t parser_inicializar(void) {
    estado = PARSER_AGUARDANDO_HEADER;
    posicao = 0U;
    pacote_disponivel = false;
    pacotes_validos = 0U;
    erros_crc = 0U;
    memset(buffer, 0, sizeof(buffer));
    memset(&ultimo_pacote, 0, sizeof(ultimo_pacote));
    return STATUS_OK;
}

parser_estado_t parser_alimentar_byte(uint8_t byte) {
    if (estado == PARSER_PACOTE_COMPLETO || estado == PARSER_ERRO_CRC) {
        estado = PARSER_AGUARDANDO_HEADER;
        posicao = 0U;
    }

    if (estado == PARSER_AGUARDANDO_HEADER) {
        if (byte != PROTOCOL_HEADER_BYTE) {
            return estado;
        }

        buffer[0] = byte;
        posicao = 1U;
        estado = PARSER_RECEBENDO_DADOS;
        pacote_disponivel = false;
        return estado;
    }

    buffer[posicao++] = byte;
    if (posicao < sizeof(buffer)) {
        return estado;
    }

    telemetry_packet_t pacote;
    memcpy(&pacote, buffer, sizeof(pacote));
    posicao = 0U;

    if (!crc16_verificar(
            buffer,
            (uint16_t)offsetof(telemetry_packet_t, crc16),
            pacote.crc16)) {
        erros_crc++;
        pacote_disponivel = false;
        estado = PARSER_ERRO_CRC;
        return estado;
    }

    ultimo_pacote = pacote;
    pacote_disponivel = true;
    pacotes_validos++;
    estado = PARSER_PACOTE_COMPLETO;
    return estado;
}

status_t parser_obter_pacote(telemetry_packet_t *pacote) {
    if (pacote == NULL || !pacote_disponivel) {
        return STATUS_ERRO_GENERICO;
    }

    *pacote = ultimo_pacote;
    pacote_disponivel = false;
    return STATUS_OK;
}

void parser_resetar(void) {
    estado = PARSER_AGUARDANDO_HEADER;
    posicao = 0U;
    pacote_disponivel = false;
}

parser_estado_t parser_obter_estado(void) {
    return estado;
}

uint32_t parser_pacotes_validos(void) {
    return pacotes_validos;
}

uint32_t parser_erros_crc(void) {
    return erros_crc;
}
