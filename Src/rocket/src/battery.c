/**
 * @file battery.c
 * @brief Monitoramento de bateria LiPo 2S via ADC.
 */

#include "battery.h"
#include "pinout.h"
#include "stm32f411_hw.h"

/* ============================================================================
 * Variáveis Internas
 * ========================================================================= */
static uint16_t tensao_atual_mv = BATERIA_TENSAO_NOMINAL_MV;

/* ============================================================================
 * Funções do Módulo
 * ========================================================================= */

status_t bateria_inicializar(void) {
    hw_gpio_analog(PINO_BATERIA_ADC_PORTA, PINO_BATERIA_ADC_PINO);
    return hw_adc1_init_channel(PINO_BATERIA_ADC_CANAL);
}

uint16_t bateria_ler_tensao_mv(void) {
    uint32_t soma = 0;
    uint16_t leitura;
    uint8_t amostras = 0;

    for (uint8_t i = 0; i < 16U; ++i) {
        if (hw_adc1_read(&leitura) == STATUS_OK) {
            soma += leitura;
            amostras++;
        }
    }

    if (amostras > 0U) {
        uint32_t media = soma / amostras;
        float tensao = ((float)media * (float)ADC_VREF_MV /
                        (float)((1UL << ADC_RESOLUCAO_BITS) - 1UL)) *
                       BATERIA_DIVISOR_RESISTIVO_FATOR;
        tensao_atual_mv = (uint16_t)tensao;
    }
    return tensao_atual_mv;
}

uint8_t bateria_ler_porcentagem(void) {
    if (tensao_atual_mv >= BATERIA_TENSAO_NOMINAL_MV) {
        return 100;
    } else if (tensao_atual_mv <= BATERIA_TENSAO_MINIMA_MV) {
        return 0;
    }

    float diff = (float)(tensao_atual_mv - BATERIA_TENSAO_MINIMA_MV);
    float range = (float)(BATERIA_TENSAO_NOMINAL_MV - BATERIA_TENSAO_MINIMA_MV);
    return (uint8_t)((diff / range) * 100.0f);
}

bool bateria_tensao_baixa(void) {
    return (tensao_atual_mv < BATERIA_TENSAO_MINIMA_MV);
}
