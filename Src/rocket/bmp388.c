/**
 * @file bmp388.c
 * @brief Implementação do driver do sensor barométrico BMP388.
 */

#include "bmp388.h"
#include "hal_plataforma.h"
#include "pinout.h"
#include "stm32f411_hw.h"
#include <math.h>

/* ============================================================================
 * Variáveis Internas
 * ========================================================================= */
static bmp388_calib_t coeficientes;
static float pressao_solo_pa = BMP388_PRESSAO_NIVEL_MAR_PA;
static bool calibracao_lida = false;
static float temperatura_linear = 0.0f;

static status_t bmp388_escrever(uint8_t reg, uint8_t valor) {
    uint8_t dados[2] = {(uint8_t)(reg & 0x7FU), valor};
    hw_gpio_write(PINO_BMP388_CS_PORTA, PINO_BMP388_CS_PINO, false);
    status_t status = hw_spi_transfer(HW_SPI1, dados, NULL, sizeof(dados));
    hw_gpio_write(PINO_BMP388_CS_PORTA, PINO_BMP388_CS_PINO, true);
    return status;
}

static status_t bmp388_ler(uint8_t reg, uint8_t *dados, uint8_t tamanho) {
    uint8_t comando[2] = {(uint8_t)(reg | 0x80U), 0xFFU};
    if (!dados || tamanho == 0U) return STATUS_ERRO_GENERICO;
    hw_gpio_write(PINO_BMP388_CS_PORTA, PINO_BMP388_CS_PINO, false);
    status_t status = hw_spi_transfer(HW_SPI1, comando, NULL, sizeof(comando));
    if (status == STATUS_OK) status = hw_spi_transfer(HW_SPI1, NULL, dados, tamanho);
    hw_gpio_write(PINO_BMP388_CS_PORTA, PINO_BMP388_CS_PINO, true);
    return status;
}

static uint16_t u16_le(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static int16_t s16_le(const uint8_t *p) {
    return (int16_t)u16_le(p);
}

static status_t bmp388_compensar(float *pressao_pa, float *temperatura_c) {
    uint8_t bruto[6];
    status_t status = bmp388_ler(BMP388_REG_PRESS_DATA, bruto, sizeof(bruto));
    if (status != STATUS_OK) return status;

    uint32_t pressao_bruta = (uint32_t)bruto[0] | ((uint32_t)bruto[1] << 8) |
                             ((uint32_t)bruto[2] << 16);
    uint32_t temperatura_bruta = (uint32_t)bruto[3] | ((uint32_t)bruto[4] << 8) |
                                 ((uint32_t)bruto[5] << 16);

    float dt = (float)temperatura_bruta - coeficientes.par_t1;
    temperatura_linear = dt * coeficientes.par_t2 + dt * dt * coeficientes.par_t3;

    float t2 = temperatura_linear * temperatura_linear;
    float t3 = t2 * temperatura_linear;
    float offset = coeficientes.par_p5 + coeficientes.par_p6 * temperatura_linear +
                   coeficientes.par_p7 * t2 + coeficientes.par_p8 * t3;
    float sensibilidade = coeficientes.par_p1 + coeficientes.par_p2 * temperatura_linear +
                          coeficientes.par_p3 * t2 + coeficientes.par_p4 * t3;
    float p = (float)pressao_bruta;
    float pressao = offset + p * sensibilidade +
                    p * p * (coeficientes.par_p9 + coeficientes.par_p10 * temperatura_linear) +
                    p * p * p * coeficientes.par_p11;

    if (temperatura_c) *temperatura_c = temperatura_linear;
    if (pressao_pa) *pressao_pa = pressao;
    return (pressao > 10000.0f && pressao < 130000.0f) ? STATUS_OK : STATUS_ERRO_SENSOR;
}

/* ============================================================================
 * Funções do Módulo
 * ========================================================================= */

status_t bmp388_inicializar(void) {
    uint8_t chip_id;
    uint8_t calib[21];
    hw_gpio_output(PINO_BMP388_CS_PORTA, PINO_BMP388_CS_PINO, true);
    status_t status = hw_spi_init(HW_SPI1, 3);
    if (status != STATUS_OK) return status;

    status = bmp388_ler(BMP388_REG_CHIP_ID, &chip_id, 1);
    if (status != STATUS_OK || chip_id != BMP388_CHIP_ID_ESPERADO) return STATUS_ERRO_SENSOR;

    status = bmp388_escrever(BMP388_REG_CMD, BMP388_CMD_SOFT_RESET);
    if (status != STATUS_OK) return status;
    plataforma_delay_ms(10);

    status = bmp388_ler(BMP388_REG_CALIB_DATA, calib, sizeof(calib));
    if (status != STATUS_OK) return status;
    coeficientes.par_t1 = (float)u16_le(&calib[0]) * 256.0f;
    coeficientes.par_t2 = (float)u16_le(&calib[2]) / 1073741824.0f;
    coeficientes.par_t3 = (float)(int8_t)calib[4] / 281474976710656.0f;
    coeficientes.par_p1 = ((float)s16_le(&calib[5]) - 16384.0f) / 1048576.0f;
    coeficientes.par_p2 = ((float)s16_le(&calib[7]) - 16384.0f) / 536870912.0f;
    coeficientes.par_p3 = (float)(int8_t)calib[9] / 4294967296.0f;
    coeficientes.par_p4 = (float)(int8_t)calib[10] / 137438953472.0f;
    coeficientes.par_p5 = (float)u16_le(&calib[11]) * 8.0f;
    coeficientes.par_p6 = (float)u16_le(&calib[13]) / 64.0f;
    coeficientes.par_p7 = (float)(int8_t)calib[15] / 256.0f;
    coeficientes.par_p8 = (float)(int8_t)calib[16] / 32768.0f;
    coeficientes.par_p9 = (float)s16_le(&calib[17]) / 281474976710656.0f;
    coeficientes.par_p10 = (float)(int8_t)calib[19] / 281474976710656.0f;
    coeficientes.par_p11 = (float)(int8_t)calib[20] / 36893488147419103232.0f;
    calibracao_lida = true;

    if (bmp388_escrever(BMP388_REG_OSR, 0x0BU) != STATUS_OK) return STATUS_ERRO_SPI;
    if (bmp388_escrever(BMP388_REG_ODR, 0x02U) != STATUS_OK) return STATUS_ERRO_SPI;
    if (bmp388_escrever(BMP388_REG_CONFIG, 0x04U) != STATUS_OK) return STATUS_ERRO_SPI;
    return bmp388_escrever(BMP388_REG_PWR_CTRL, 0x33U);
}

status_t bmp388_ler_pressao(float *pressao_pa) {
    if (!calibracao_lida || !pressao_pa) return STATUS_ERRO_SENSOR;
    return bmp388_compensar(pressao_pa, NULL);
}

status_t bmp388_ler_temperatura(float *temperatura_c) {
    if (!calibracao_lida || !temperatura_c) return STATUS_ERRO_SENSOR;
    return bmp388_compensar(NULL, temperatura_c);
}

status_t bmp388_calcular_altitude(float pressao_pa, float *altitude_m) {
    if (!altitude_m || pressao_pa <= 0.0f || pressao_solo_pa <= 0.0f) return STATUS_ERRO_GENERICO;

    /* Fórmula barométrica simplificada: h = 44330 * (1 - (P/P0)^(1/5.255)) */
    *altitude_m = 44330.0f * (1.0f - powf(pressao_pa / pressao_solo_pa, 0.1902949f));

    return STATUS_OK;
}

status_t bmp388_calibrar_zero(void) {
    float pressao_media = 0.0f;
    float pressao_leitura;
    int num_amostras = 50;
    int amostras_sucesso = 0;

    for (int i = 0; i < num_amostras; i++) {
        if (bmp388_ler_pressao(&pressao_leitura) == STATUS_OK) {
            pressao_media += pressao_leitura;
            amostras_sucesso++;
        }
        plataforma_delay_ms(10);
    }

    if (amostras_sucesso > 0) {
        pressao_solo_pa = pressao_media / (float)amostras_sucesso;
        return STATUS_OK;
    }

    return STATUS_ERRO_SENSOR;
}

status_t bmp388_autoteste(void) {
    uint8_t chip_id;
    float pressao;
    if (bmp388_ler(BMP388_REG_CHIP_ID, &chip_id, 1) != STATUS_OK ||
        chip_id != BMP388_CHIP_ID_ESPERADO) {
        return STATUS_ERRO_SENSOR;
    }
    return bmp388_ler_pressao(&pressao);
}
