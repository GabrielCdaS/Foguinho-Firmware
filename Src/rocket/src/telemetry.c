/**
 * @file telemetry.c
 * @brief Implementação do módulo de telemetria via LoRa.
 */

#include "telemetry.h"
#include "sx1278.h"
#include "crc16.h"
#include "hal_plataforma.h"
#include "fusion.h"
#include <limits.h>
#include <math.h>

static uint8_t pacote_id_sequencial = 0;
static uint32_t total_pacotes_enviados = 0;
static uint32_t ultimo_envio_ms = 0;

static telemetry_packet_t pacote_atual;

static int32_t coordenada_escalada(float coordenada) {
    if (!isfinite(coordenada)) return 0;
    double escalada = (double)coordenada * 10000000.0;
    if (escalada > INT32_MAX) return INT32_MAX;
    if (escalada < INT32_MIN) return INT32_MIN;
    return (int32_t)escalada;
}

static int16_t altitude_gps_escalada(float altitude_m) {
    if (!isfinite(altitude_m)) return 0;
    if (altitude_m > (float)INT16_MAX) return INT16_MAX;
    if (altitude_m < (float)INT16_MIN) return INT16_MIN;
    return (int16_t)altitude_m;
}

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
    pacote->acceleration_g = sqrtf(
        dados->aceleracao_x_g * dados->aceleracao_x_g +
        dados->aceleracao_y_g * dados->aceleracao_y_g +
        dados->aceleracao_z_g * dados->aceleracao_z_g
    );

    pacote->battery_mv = (uint16_t)dados->tensao_bateria_mv;
    pacote->flight_state = (uint8_t)estado;

    pacote->gps_latitude = coordenada_escalada(dados->latitude);
    pacote->gps_longitude = coordenada_escalada(dados->longitude);
    pacote->gps_altitude_m = altitude_gps_escalada(dados->gps_altitude_m);

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
