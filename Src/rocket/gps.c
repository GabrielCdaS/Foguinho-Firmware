/**
 * @file gps.c
 * @brief Implementação do módulo GPS.
 */

#include "gps.h"
#include "hal_plataforma.h"
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
static uint32_t last_fix_ms = 0;
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

static bool coordenada_decimal(const char *valor, char hemisferio, float *resultado) {
    char *fim;
    if (!valor || !resultado || *valor == '\0') return false;
    float nmea = strtof(valor, &fim);
    if (fim == valor || *fim != '\0' || nmea < 0.0f) return false;
    int graus = (int)(nmea / 100.0f);
    float minutos = nmea - (float)(graus * 100);
    if (minutos < 0.0f || minutos >= 60.0f) return false;
    float decimal = (float)graus + minutos / 60.0f;
    if (hemisferio == 'S' || hemisferio == 'W') decimal = -decimal;
    else if (hemisferio != 'N' && hemisferio != 'E') return false;
    float limite = (hemisferio == 'N' || hemisferio == 'S') ? 90.0f : 180.0f;
    if (decimal < -limite || decimal > limite) return false;
    *resultado = decimal;
    return true;
}

static uint8_t separar_campos(char *linha, char **campos, uint8_t capacidade) {
    uint8_t quantidade = 0;
    char *cursor = linha;
    while (quantidade < capacidade) {
        campos[quantidade++] = cursor;
        char *virgula = strchr(cursor, ',');
        if (!virgula) break;
        *virgula = '\0';
        cursor = virgula + 1;
    }
    return quantidade;
}

static void processar_gga(char *linha) {
    char *campos[15] = {0};
    uint8_t quantidade = separar_campos(linha, campos, 15U);
    if (quantidade < 10U) return;

    int qualidade = atoi(campos[6]);
    int satelites = atoi(campos[7]);
    last_sats = (uint8_t)(satelites < 0 ? 0 : (satelites > 127 ? 127 : satelites));
    last_fix = qualidade > 0;
    if (!last_fix) return;

    float latitude;
    float longitude;
    char *fim_altitude;
    float altitude = strtof(campos[9], &fim_altitude);
    if (!coordenada_decimal(campos[2], campos[3][0], &latitude) ||
        !coordenada_decimal(campos[4], campos[5][0], &longitude) ||
        fim_altitude == campos[9] || *fim_altitude != '\0') {
        last_fix = false;
        return;
    }

    last_lat = latitude;
    last_lon = longitude;
    last_alt = altitude;
    last_fix_ms = plataforma_obter_tick_ms();
}

static void processar_rmc(char *linha) {
    char *campos[13] = {0};
    uint8_t quantidade = separar_campos(linha, campos, 13U);
    if (quantidade < 7U || campos[2][0] != 'A') {
        last_fix = false;
        return;
    }

    float latitude;
    float longitude;
    if (!coordenada_decimal(campos[3], campos[4][0], &latitude) ||
        !coordenada_decimal(campos[5], campos[6][0], &longitude)) {
        last_fix = false;
        return;
    }

    last_lat = latitude;
    last_lon = longitude;
    last_fix = true;
    last_fix_ms = plataforma_obter_tick_ms();
}

status_t gps_processar_sentenca_nmea(const char *sentenca_nmea) {
    char linha[GPS_SENTENCE_SIZE];
    size_t tamanho;
    if (!sentenca_nmea) return STATUS_ERRO_GENERICO;

    tamanho = strlen(sentenca_nmea);
    while (tamanho > 0U &&
           (sentenca_nmea[tamanho - 1U] == '\r' || sentenca_nmea[tamanho - 1U] == '\n')) {
        tamanho--;
    }
    if (tamanho == 0U || tamanho >= sizeof(linha)) return STATUS_ERRO_GENERICO;
    memcpy(linha, sentenca_nmea, tamanho);
    linha[tamanho] = '\0';

    if (!checksum_valido(linha)) return STATUS_ERRO_CRC;
    if (strncmp(linha, "$GPGGA,", 7) == 0 || strncmp(linha, "$GNGGA,", 7) == 0) {
        processar_gga(linha);
        return STATUS_OK;
    }
    if (strncmp(linha, "$GPRMC,", 7) == 0 || strncmp(linha, "$GNRMC,", 7) == 0) {
        processar_rmc(linha);
        return STATUS_OK;
    }
    return STATUS_ERRO_GENERICO;
}

status_t gps_inicializar(void) {
    rx_head = 0;
    rx_tail = 0;
    sentence_length = 0;
    last_lat = 0.0f;
    last_lon = 0.0f;
    last_alt = 0.0f;
    last_sats = 0;
    last_fix = false;
    last_fix_ms = 0;
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
            gps_processar_sentenca_nmea(sentence);
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
    uint32_t agora = plataforma_obter_tick_ms();
    bool fix_atual = last_fix && last_fix_ms != 0U &&
                     (agora - last_fix_ms) <= GPS_FIX_TIMEOUT_MS;
    if (latitude) *latitude = last_lat;
    if (longitude) *longitude = last_lon;
    if (altitude) *altitude = last_alt;
    if (satelites) *satelites = last_sats;
    if (fix_valido) *fix_valido = fix_atual;

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
