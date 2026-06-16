#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "command.h"
#include "crc16.h"
#include "protocol.h"
#include "protocol_defs.h"
#include "sx1278.h"
#include "telemetry_parser.h"
#include "usb_bridge.h"

static uint8_t radio_rx[PROTOCOL_PACKET_SIZE];
static uint8_t radio_rx_size;
static bool radio_available;
static uint8_t radio_tx[PROTOCOL_PACKET_SIZE];
static uint8_t radio_tx_size;
static uint8_t usb_rx[32];
static uint16_t usb_rx_size;
static uint8_t usb_tx[PROTOCOL_PACKET_SIZE];
static uint16_t usb_tx_size;

status_t sx1278_transmitir(const uint8_t *dados, uint8_t tamanho) {
    memcpy(radio_tx, dados, tamanho);
    radio_tx_size = tamanho;
    return STATUS_OK;
}

status_t sx1278_receber(uint8_t *buffer, uint8_t *tamanho, uint32_t timeout_ms) {
    (void)timeout_ms;
    if (!radio_available) return STATUS_ERRO_TIMEOUT;
    memcpy(buffer, radio_rx, radio_rx_size);
    *tamanho = radio_rx_size;
    radio_available = false;
    return STATUS_OK;
}

bool sx1278_pacote_disponivel(void) {
    return radio_available;
}

status_t usb_bridge_enviar(const uint8_t *dados, uint16_t tamanho) {
    memcpy(usb_tx, dados, tamanho);
    usb_tx_size = tamanho;
    return STATUS_OK;
}

status_t usb_bridge_enviar_pacote_telemetria(const telemetry_packet_t *pacote) {
    return usb_bridge_enviar((const uint8_t *)pacote, sizeof(*pacote));
}

bool usb_bridge_dados_disponiveis(void) {
    return usb_rx_size > 0U;
}

status_t usb_bridge_receber(uint8_t *buffer, uint16_t *tamanho) {
    memcpy(buffer, usb_rx, usb_rx_size);
    *tamanho = usb_rx_size;
    usb_rx_size = 0U;
    return STATUS_OK;
}

static telemetry_packet_t criar_telemetria(uint8_t id) {
    telemetry_packet_t pacote = {0};
    pacote.header = PROTOCOL_HEADER_BYTE;
    pacote.packet_id = id;
    pacote.altitude_m = 42.5f;
    pacote.crc16 = crc16_calculate(
        (const uint8_t *)&pacote,
        (uint16_t)offsetof(telemetry_packet_t, crc16));
    return pacote;
}

static command_packet_t criar_comando(command_id_t id, uint8_t payload) {
    command_packet_t pacote = {
        .header = PROTOCOL_HEADER_BYTE,
        .command_id = (uint8_t)id,
        .payload = payload,
        .crc16 = 0U
    };
    pacote.crc16 = crc16_calculate(
        (const uint8_t *)&pacote,
        (uint16_t)offsetof(command_packet_t, crc16));
    return pacote;
}

static void colocar_no_radio(const void *dados, uint8_t tamanho) {
    memcpy(radio_rx, dados, tamanho);
    radio_rx_size = tamanho;
    radio_available = true;
}

static void testar_parser(void) {
    telemetry_packet_t entrada = criar_telemetria(7U);
    telemetry_packet_t saida;

    assert(parser_inicializar() == STATUS_OK);
    const uint8_t *bytes = (const uint8_t *)&entrada;
    for (size_t i = 0U; i < sizeof(entrada); ++i) {
        parser_alimentar_byte(bytes[i]);
    }
    assert(parser_obter_estado() == PARSER_PACOTE_COMPLETO);
    assert(parser_obter_pacote(&saida) == STATUS_OK);
    assert(saida.packet_id == 7U);
    assert(parser_pacotes_validos() == 1U);

    entrada.altitude_m += 1.0f;
    bytes = (const uint8_t *)&entrada;
    for (size_t i = 0U; i < sizeof(entrada); ++i) {
        parser_alimentar_byte(bytes[i]);
    }
    assert(parser_obter_estado() == PARSER_ERRO_CRC);
    assert(parser_erros_crc() == 1U);
}

static void testar_ponte(void) {
    assert(protocolo_base_inicializar() == STATUS_OK);

    telemetry_packet_t pacote = criar_telemetria(10U);
    colocar_no_radio(&pacote, sizeof(pacote));
    assert(protocolo_base_processar_radio() == STATUS_OK);
    assert(usb_tx_size == sizeof(pacote));
    assert(protocolo_base_pacotes_recebidos() == 1U);

    pacote = criar_telemetria(13U);
    colocar_no_radio(&pacote, sizeof(pacote));
    assert(protocolo_base_processar_radio() == STATUS_OK);
    assert(protocolo_base_pacotes_perdidos() == 2U);

    command_packet_t comando = criar_comando(CMD_ARM, 0U);
    memcpy(usb_rx, &comando, sizeof(comando));
    usb_rx_size = sizeof(comando);
    assert(protocolo_base_processar_usb() == STATUS_OK);
    assert(radio_tx_size == sizeof(comando));
    assert(memcmp(radio_tx, &comando, sizeof(comando)) == 0);
}

static void testar_comandos(void) {
    command_packet_t resposta;

    assert(comando_inicializar() == STATUS_OK);
    assert(comando_enviar_ping() == STATUS_OK);
    assert(comando_aguardando_resposta());
    assert(comando_comandos_enviados() == 1U);

    resposta = criar_comando(CMD_PING, COMMAND_RESULT_OK);
    colocar_no_radio(&resposta, sizeof(resposta));
    assert(comando_verificar_resposta(100U) == STATUS_OK);
    assert(!comando_aguardando_resposta());
}

int main(void) {
    testar_parser();
    testar_ponte();
    testar_comandos();
    puts("base station tests passed");
    return 0;
}
