/**
 * @file crc16.c
 * @brief Implementação de cálculo e verificação de CRC-16/CCITT.
 */

#include "crc16.h"

uint16_t crc16_calculate(const uint8_t *dados, uint16_t tamanho) {
    uint16_t crc = CRC16_VALOR_INICIAL;

    for (uint16_t i = 0; i < tamanho; i++) {
        crc ^= (uint16_t)(dados[i] << 8);
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ CRC16_POLINOMIO;
            } else {
                crc = (crc << 1);
            }
        }
    }

    return crc;
}

bool crc16_verificar(const uint8_t *dados, uint16_t tamanho, uint16_t crc_esperado) {
    uint16_t crc_calculado = crc16_calculate(dados, tamanho);
    return (crc_calculado == crc_esperado);
}
