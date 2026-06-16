/**
 * @file protocol.c
 * @brief Roteamento de pacotes entre LoRa e USB CDC.
 */

#include "protocol.h"

#include "config.h"
#include "crc16.h"
#include "sx1278.h"
#include "telemetry_parser.h"
#include "usb_bridge.h"

#include <stddef.h>
#include <string.h>

static uint32_t pacotes_recebidos;
static uint32_t pacotes_invalidos;
static uint32_t pacotes_perdidos;
static uint8_t ultimo_id;
static bool possui_ultimo_id;

static bool comando_valido(const command_packet_t *pacote) {
    return pacote->header == PROTOCOL_HEADER_BYTE &&
           pacote->command_id >= CMD_ARM &&
           pacote->command_id <= CMD_STOP_LOG &&
           crc16_verificar(
               (const uint8_t *)pacote,
               (uint16_t)offsetof(command_packet_t, crc16),
               pacote->crc16);
}

status_t protocolo_base_inicializar(void) {
    pacotes_recebidos = 0U;
    pacotes_invalidos = 0U;
    pacotes_perdidos = 0U;
    ultimo_id = 0U;
    possui_ultimo_id = false;
    return parser_inicializar();
}

status_t protocolo_base_processar_radio(void) {
    uint8_t buffer[TAMANHO_BUFFER_RADIO_RX];
    uint8_t tamanho = 0U;

    if (!sx1278_pacote_disponivel()) {
        return STATUS_OK;
    }

    status_t resultado = sx1278_receber(buffer, &tamanho, 0U);
    if (resultado != STATUS_OK) {
        if (resultado == STATUS_ERRO_CRC) {
            pacotes_invalidos++;
        }
        return resultado;
    }

    if (tamanho == sizeof(command_packet_t)) {
        command_packet_t resposta;
        memcpy(&resposta, buffer, sizeof(resposta));
        if (!comando_valido(&resposta)) {
            pacotes_invalidos++;
            return STATUS_ERRO_CRC;
        }
        return usb_bridge_enviar(buffer, tamanho);
    }

    if (tamanho != sizeof(telemetry_packet_t)) {
        pacotes_invalidos++;
        return STATUS_ERRO_RADIO;
    }

    parser_estado_t estado = PARSER_AGUARDANDO_HEADER;
    for (uint8_t i = 0U; i < tamanho; ++i) {
        estado = parser_alimentar_byte(buffer[i]);
    }
    if (estado != PARSER_PACOTE_COMPLETO) {
        pacotes_invalidos++;
        return estado == PARSER_ERRO_CRC ? STATUS_ERRO_CRC : STATUS_ERRO_RADIO;
    }

    telemetry_packet_t pacote;
    if (parser_obter_pacote(&pacote) != STATUS_OK) {
        pacotes_invalidos++;
        return STATUS_ERRO_GENERICO;
    }

    if (possui_ultimo_id) {
        uint8_t diferenca = (uint8_t)(pacote.packet_id - ultimo_id);
        if (diferenca > 1U) {
            pacotes_perdidos += (uint32_t)(diferenca - 1U);
        }
    }
    ultimo_id = pacote.packet_id;
    possui_ultimo_id = true;
    pacotes_recebidos++;

    return usb_bridge_enviar_pacote_telemetria(&pacote);
}

status_t protocolo_base_processar_usb(void) {
    uint8_t buffer[TAMANHO_BUFFER_USB_RX];
    uint16_t tamanho = 0U;

    if (!usb_bridge_dados_disponiveis()) {
        return STATUS_OK;
    }

    status_t resultado = usb_bridge_receber(buffer, &tamanho);
    if (resultado != STATUS_OK) {
        return resultado;
    }
    if (tamanho == 0U || (tamanho % sizeof(command_packet_t)) != 0U) {
        return STATUS_ERRO_USB;
    }

    for (uint16_t offset = 0U; offset < tamanho; offset += sizeof(command_packet_t)) {
        command_packet_t pacote;
        memcpy(&pacote, &buffer[offset], sizeof(pacote));
        if (!comando_valido(&pacote)) {
            return STATUS_ERRO_CRC;
        }

        resultado = sx1278_transmitir(
            &buffer[offset],
            (uint8_t)sizeof(command_packet_t));
        if (resultado != STATUS_OK) {
            return resultado;
        }
    }

    return STATUS_OK;
}

uint32_t protocolo_base_pacotes_recebidos(void) {
    return pacotes_recebidos;
}

uint32_t protocolo_base_pacotes_invalidos(void) {
    return pacotes_invalidos;
}

uint32_t protocolo_base_pacotes_perdidos(void) {
    return pacotes_perdidos;
}
