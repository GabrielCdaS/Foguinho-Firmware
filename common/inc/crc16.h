/**
 * @file crc16.h
 * @brief Funções de cálculo e verificação de CRC-16/CCITT.
 *
 * Utiliza o polinômio CRC-16/CCITT (0x1021) com valor inicial 0xFFFF.
 * Empregado para verificação de integridade dos pacotes de telemetria
 * e de comando transmitidos via rádio LoRa.
 *
 * Compartilhado entre o firmware do foguete e da estação base.
 */

#ifndef CRC16_H
#define CRC16_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================
 * Constantes do CRC-16/CCITT
 * ============================================================ */

/** Polinômio gerador CRC-16/CCITT */
#define CRC16_POLINOMIO       (0x1021)

/** Valor inicial do registrador CRC */
#define CRC16_VALOR_INICIAL   (0xFFFF)

/* ============================================================
 * Protótipos das funções
 * ============================================================ */

/**
 * @brief Calcula o CRC-16/CCITT sobre um bloco de dados.
 *
 * Percorre cada byte do buffer, aplicando o polinômio 0x1021
 * bit a bit a partir do valor inicial 0xFFFF.
 *
 * @param dados    Ponteiro para o buffer de dados.
 * @param tamanho  Quantidade de bytes a processar.
 * @return         Valor CRC-16 calculado (16 bits).
 */
uint16_t crc16_calculate(const uint8_t *dados, uint16_t tamanho);

/**
 * @brief Verifica se o CRC-16 calculado confere com o esperado.
 *
 * Recalcula o CRC sobre o buffer fornecido e compara com o
 * valor de referência. Útil para validar pacotes recebidos.
 *
 * @param dados        Ponteiro para o buffer de dados.
 * @param tamanho      Quantidade de bytes a processar.
 * @param crc_esperado Valor CRC-16 esperado para comparação.
 * @return             true se o CRC calculado for igual ao esperado,
 *                     false caso contrário.
 */
bool crc16_verificar(const uint8_t *dados, uint16_t tamanho, uint16_t crc_esperado);

#endif /* CRC16_H */
