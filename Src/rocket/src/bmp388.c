/**
 * @file bmp388.c
 * @brief Implementação do driver do sensor barométrico BMP388.
 */

#include "bmp388.h"
#include <math.h>

/* ============================================================================
 * Variáveis Internas
 * ========================================================================= */
static bmp388_calib_t coeficientes;
static float pressao_solo_pa = BMP388_PRESSAO_NIVEL_MAR_PA;
static bool calibracao_lida = false;

/* ============================================================================
 * Funções do Módulo
 * ========================================================================= */

status_t bmp388_inicializar(void) {
    /* TODO: Configurar SPI1, PA4 (CS) */

    /* TODO: Ler CHIP_ID e verificar se é BMP388_CHIP_ID_ESPERADO */

    /* TODO: Ler coeficientes de calibração do BMP388_REG_CALIB_DATA */
    /* Como não temos as funções SPI de hardware escritas e não temos CMSIS,
       vamos simular que a leitura ocorreu com sucesso. */
    calibracao_lida = true;

    /* TODO: Configurar Oversampling e Filtro IIR */

    /* TODO: Habilitar modo normal em BMP388_REG_PWR_CTRL */

    return STATUS_OK;
}

status_t bmp388_ler_pressao(float *pressao_pa) {
    if (!calibracao_lida) return STATUS_ERRO_SENSOR;

    /* TODO: Ler registradores BMP388_REG_PRESS_DATA via SPI */
    /* Simulação de dados: 101325 Pa */
    if (pressao_pa) {
        *pressao_pa = 101325.0f;
    }

    return STATUS_OK;
}

status_t bmp388_ler_temperatura(float *temperatura_c) {
    if (!calibracao_lida) return STATUS_ERRO_SENSOR;

    /* TODO: Ler registradores BMP388_REG_TEMP_DATA via SPI */
    /* Simulação de dados: 25.0 ºC */
    if (temperatura_c) {
        *temperatura_c = 25.0f;
    }

    return STATUS_OK;
}

status_t bmp388_calcular_altitude(float pressao_pa, float *altitude_m) {
    if (!altitude_m) return STATUS_ERRO_GENERICO;

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
        /* hal_delay_ms(10); // se necessário */
    }

    if (amostras_sucesso > 0) {
        pressao_solo_pa = pressao_media / (float)amostras_sucesso;
        return STATUS_OK;
    }

    return STATUS_ERRO_SENSOR;
}

status_t bmp388_autoteste(void) {
    /* TODO: Ler CHIP_ID e verificar resposta */
    return STATUS_OK;
}
