/**
 * @file telemetry.c
 * @brief Implementação do módulo de telemetria via LoRa.
 */

#include "telemetry.h"
#include "sx1278.h"
#include "crc16.h"
#include "hal_plataforma.h"
#include "fusion.h"

static uint8_t pacote_id_sequencial = 0;
static uint32_t total_pacotes_enviados = 0;
static uint32_t ultimo_envio_ms = 0;

static telemetry_packet_t pacote_atual;

status_t telemetria_inicializar(void) {
    pacote_id_sequencial = 0;
    total_pacotes_enviados = 0;
    ultimo_envio_ms = 0;
    return STATUS_OK;
}

status_t telemetria_construir_pacote(telemetry_packet_t *pacote,
                                     const dados_sensores_t *dados,
                                     flight_state_t estado) {
    if (!pacote || !dados) return STATUS_ERRO_GENERICO;

    pacote->header = PROTOCOL_HEADER_BYTE;
    pacote->packet_id = pacote_id_sequencial++;
    pacote->timestamp_ms = dados->timestamp_ms;

    pacote->altitude_m = fusao_obter_altitude();
    pacote->vert_velocity_ms = fusao_obter_velocidade_vertical();
    pacote->acceleration_g = dados->aceleracao_z_g;

    pacote->battery_mv = (uint16_t)dados->tensao_bateria_mv;
    pacote->flight_state = (uint8_t)estado;

    pacote->gps_latitude = (int32_t)(dados->latitude * 10000000.0f);
    pacote->gps_longitude = (int32_t)(dados->longitude * 10000000.0f);
    pacote->gps_altitude_m = (int16_t)dados->gps_altitude_m;

    pacote->gps_info = (dados->gps_satellites & 0x7F) | (dados->gps_fix_valid ? 0x80 : 0x00);

    /* Calcular CRC excluindo o próprio campo de CRC (2 bytes finais) */
    pacote->crc16 = crc16_calculate((uint8_t*)pacote, sizeof(telemetry_packet_t) - 2);

    /* Salvar globalmente para o envio */
    pacote_atual = *pacote;

    return STATUS_OK;
}

status_t telemetria_enviar(void) {
    status_t res = sx1278_transmitir((uint8_t*)&pacote_atual, sizeof(telemetry_packet_t));
    if (res == STATUS_OK) {
        total_pacotes_enviados++;
        ultimo_envio_ms = plataforma_obter_tick_ms();
    }
    return res;
}

uint8_t telemetria_obter_id_pacote(void) {
    return pacote_id_sequencial;
}

uint32_t telemetria_pacotes_enviados(void) {
    return total_pacotes_enviados;
}

uint32_t telemetria_ultimo_envio_ms(void) {
    return ultimo_envio_ms;
}
