/**
 * @file sx1278.h
 * @brief Fachada do projeto para o driver oficial SX1276/77/78/79 da Semtech.
 */

#ifndef SX1278_H
#define SX1278_H

#include <stdbool.h>
#include <stdint.h>

#include "sensors.h"

#define SX1278_FREQUENCIA_433MHZ   433000000UL
#define SX1278_FREQUENCIA_PADRAO   SX1278_FREQUENCIA_433MHZ
#define SX1278_POTENCIA_PADRAO_DBM 17
#define SX1278_TAMANHO_MAX_PACOTE  255

typedef enum {
    SX1278_SF7 = 7,
    SX1278_SF8,
    SX1278_SF9,
    SX1278_SF10,
    SX1278_SF11,
    SX1278_SF12
} sx1278_sf_t;

typedef enum {
    SX1278_BW_7K8 = 0,
    SX1278_BW_10K4,
    SX1278_BW_15K6,
    SX1278_BW_20K8,
    SX1278_BW_31K25,
    SX1278_BW_41K7,
    SX1278_BW_62K5,
    SX1278_BW_125K,
    SX1278_BW_250K,
    SX1278_BW_500K
} sx1278_bw_t;

typedef enum {
    SX1278_CR_4_5 = 1,
    SX1278_CR_4_6,
    SX1278_CR_4_7,
    SX1278_CR_4_8
} sx1278_cr_t;

#define SX1278_SF_PADRAO SX1278_SF7
#define SX1278_BW_PADRAO SX1278_BW_125K
#define SX1278_CR_PADRAO SX1278_CR_4_5

status_t sx1278_inicializar(void);
status_t sx1278_transmitir(const uint8_t *dados, uint8_t tamanho);
status_t sx1278_processar(void);
status_t sx1278_receber(uint8_t *buffer, uint8_t *tamanho, uint32_t timeout_ms);
status_t sx1278_configurar_frequencia(uint32_t frequencia_hz);
status_t sx1278_configurar_potencia(int8_t potencia_dbm);
status_t sx1278_configurar_lora(sx1278_sf_t sf, sx1278_bw_t bw, sx1278_cr_t cr);
int16_t sx1278_ler_rssi(void);
status_t sx1278_modo_sleep(void);
status_t sx1278_modo_standby(void);

#endif
