/**
 * @file gps.c
 * @brief Implementação do módulo GPS.
 */

#include "gps.h"
#include "stm32f411_hw.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define GPS_RX_BUFFER_SIZE 1024U
#define GPS_SENTENCE_SIZE  96U

static float last_lat = 0.0f;
static float last_lon = 0.0f;
static float last_alt = 0.0f;
static uint8_t last_sats = 0;
static bool last_fix = false;
static volatile uint8_t rx_buffer[GPS_RX_BUFFER_SIZE];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;
static char sentence[GPS_SENTENCE_SIZE];
static uint8_t sentence_length = 0;

static int hex_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    c = (char)toupper((unsigned char)c);
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static bool checksum_valido(const char *nmea) {
    const char *asterisco = strchr(nmea, '*');
    uint8_t checksum = 0;
    if (!asterisco || asterisco - nmea < 2 || asterisco[1] == '\0' || asterisco[2] == '\0') return false;
    for (const char *p = nmea + 1; p < asterisco; ++p) checksum ^= (uint8_t)*p;
    int alto = hex_value(asterisco[1]);
    int baixo = hex_value(asterisco[2]);
    return alto >= 0 && baixo >= 0 && checksum == (uint8_t)((alto << 4) | baixo);
}

static float coordenada_decimal(const char *valor, char hemisferio) {
    if (!valor || *valor == '\0') return 0.0f;
    float nmea = strtof(valor, NULL);
    int graus = (int)(nmea / 100.0f);
    float decimal = (float)graus + (nmea - (float)(graus * 100)) / 60.0f;
    if (hemisferio == 'S' || hemisferio == 'W') decimal = -decimal;
    return decimal;
}

static void processar_gga(char *linha) {
    char *campos[15] = {0};
    uint8_t quantidade = 0;
    char *cursor = linha;
    while (quantidade < 15U) {
        campos[quantidade++] = cursor;
        char *virgula = strchr(cursor, ',');
        if (!virgula) break;
        *virgula = '\0';
        cursor = virgula + 1;
    }
    if (quantidade < 10U) return;
    int qualidade = atoi(campos[6]);
    last_fix = qualidade > 0;
    last_sats = (uint8_t)atoi(campos[7]);
    last_lat = coordenada_decimal(campos[2], campos[3][0]);
    last_lon = coordenada_decimal(campos[4], campos[5][0]);
    last_alt = strtof(campos[9], NULL);
}

status_t gps_inicializar(void) {
    rx_head = 0;
    rx_tail = 0;
    sentence_length = 0;
    return hw_usart2_init(GPS_BAUDRATE);
}

status_t gps_processar(void) {
    while (rx_tail != rx_head) {
        char c = (char)rx_buffer[rx_tail];
        rx_tail = (uint16_t)((rx_tail + 1U) % GPS_RX_BUFFER_SIZE);

        if (c == '$') {
            sentence_length = 0;
            sentence[sentence_length++] = c;
        } else if (sentence_length > 0U && c == '\n') {
            sentence[sentence_length] = '\0';
            if (checksum_valido(sentence) &&
                (strncmp(sentence, "$GPGGA,", 7) == 0 || strncmp(sentence, "$GNGGA,", 7) == 0)) {
                processar_gga(sentence);
            }
            sentence_length = 0;
        } else if (sentence_length > 0U && c != '\r') {
            if (sentence_length < GPS_SENTENCE_SIZE - 1U) {
                sentence[sentence_length++] = c;
            } else {
                sentence_length = 0;
            }
        }
    }
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

void USART2_IRQHandler(void) {
    while (hw_usart2_rx_ready()) {
        uint8_t byte = hw_usart2_read();
        uint16_t proximo = (uint16_t)((rx_head + 1U) % GPS_RX_BUFFER_SIZE);
        if (proximo != rx_tail) {
            rx_buffer[rx_head] = byte;
            rx_head = proximo;
        }
    }
}
