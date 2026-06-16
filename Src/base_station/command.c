/**
 * @file command.c
 * @brief Construção e transmissão de comandos pela estação base.
 */

#include "command.h"

#include "crc16.h"
#include "sx1278.h"

#include <stddef.h>
#include <string.h>

static bool aguardando_resposta;
static command_id_t ultimo_comando;
static uint32_t comandos_enviados;

static bool comando_valido(command_id_t comando) {
    return comando >= CMD_ARM && comando <= CMD_STOP_LOG;
}

status_t comando_inicializar(void) {
    aguardando_resposta = false;
    ultimo_comando = (command_id_t)0;
    comandos_enviados = 0U;
    return STATUS_OK;
}

status_t comando_enviar_generico(command_id_t cmd) {
    if (!comando_valido(cmd)) {
        return STATUS_ERRO_GENERICO;
    }

    command_packet_t pacote = {
        .header = PROTOCOL_HEADER_BYTE,
        .command_id = (uint8_t)cmd,
        .payload = 0U,
        .crc16 = 0U
    };
    pacote.crc16 = crc16_calculate(
        (const uint8_t *)&pacote,
        (uint16_t)offsetof(command_packet_t, crc16));

    status_t resultado = sx1278_transmitir(
        (const uint8_t *)&pacote,
        (uint8_t)sizeof(pacote));
    if (resultado == STATUS_OK) {
        ultimo_comando = cmd;
        aguardando_resposta = true;
        comandos_enviados++;
    }
    return resultado;
}

status_t comando_enviar_arm(void) {
    return comando_enviar_generico(CMD_ARM);
}

status_t comando_enviar_disarm(void) {
    return comando_enviar_generico(CMD_DISARM);
}

status_t comando_enviar_ping(void) {
    return comando_enviar_generico(CMD_PING);
}

status_t comando_enviar_iniciar_log(void) {
    return comando_enviar_generico(CMD_START_LOG);
}

status_t comando_enviar_parar_log(void) {
    return comando_enviar_generico(CMD_STOP_LOG);
}

bool comando_aguardando_resposta(void) {
    return aguardando_resposta;
}

status_t comando_verificar_resposta(uint32_t timeout_ms) {
    uint8_t buffer[sizeof(command_packet_t)];
    uint8_t tamanho = 0U;

    if (!aguardando_resposta) {
        return STATUS_ERRO_GENERICO;
    }

    status_t resultado = sx1278_receber(buffer, &tamanho, timeout_ms);
    if (resultado != STATUS_OK) {
        return resultado;
    }
    if (tamanho != sizeof(command_packet_t)) {
        return STATUS_ERRO_RADIO;
    }

    command_packet_t resposta;
    memcpy(&resposta, buffer, sizeof(resposta));
    if (resposta.header != PROTOCOL_HEADER_BYTE ||
        resposta.command_id != (uint8_t)ultimo_comando ||
        !crc16_verificar(
            buffer,
            (uint16_t)offsetof(command_packet_t, crc16),
            resposta.crc16)) {
        return STATUS_ERRO_CRC;
    }

    aguardando_resposta = false;
    return resposta.payload == COMMAND_RESULT_OK
        ? STATUS_OK
        : STATUS_ERRO_GENERICO;
}

uint32_t comando_comandos_enviados(void) {
    return comandos_enviados;
}
