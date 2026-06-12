/**
 * @file gps.c
 * @brief Implementação do módulo GPS.
 */

#include "gps.h"
#include <string.h>
#include <stdlib.h>

static float last_lat = 0.0f;
static float last_lon = 0.0f;
static float last_alt = 0.0f;
static uint8_t last_sats = 0;
static bool last_fix = false;

status_t gps_inicializar(void) {
    /* TODO: Configurar USART2 (TX PA2, RX PA3) com baudrate GPS_BAUDRATE */
    /* TODO: Configurar DMA para recepção circular contínua */
    return STATUS_OK;
}

status_t gps_processar(void) {
    /* TODO: Processar o buffer DMA, procurar por $GPGGA ou $GNRMC */
    /* Exemplo simulação: parser NMEA para extrair latitude, longitude, altitude */
    /* Por enquanto, não alteramos os dados. Ficam os últimos conhecidos. */
    return STATUS_OK;
}

status_t gps_obter_dados(float *latitude, float *longitude, float *altitude, uint8_t *satelites, bool *fix_valido) {
    if (latitude) *latitude = last_lat;
    if (longitude) *longitude = last_lon;
    if (altitude) *altitude = last_alt;
    if (satelites) *satelites = last_sats;
    if (fix_valido) *fix_valido = last_fix;

    return STATUS_OK;
}
