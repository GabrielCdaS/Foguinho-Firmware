/**
 * @file battery.c
 * @brief Monitoramento de bateria LiPo 2S via ADC.
 */

#include "battery.h"

/* ============================================================================
 * Variáveis Internas
 * ========================================================================= */
static uint16_t tensao_atual_mv = BATERIA_TENSAO_NOMINAL_MV;

/* ============================================================================
 * Funções do Módulo
 * ========================================================================= */

status_t bateria_inicializar(void) {
    /* TODO: Configurar o clock do ADC1 e GPIOA (PA0 como analógico) */
    /* Exemplo:
     * RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
     * RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
     * GPIOA->MODER |= GPIO_MODER_MODE0; // Analog mode
     * ADC1->CR2 |= ADC_CR2_ADON;
     */
    return STATUS_OK;
}

uint16_t bateria_ler_tensao_mv(void) {
    /* TODO: Iniciar conversão ADC e aguardar EOC */
    /* Exemplo:
     * ADC1->CR2 |= ADC_CR2_SWSTART;
     * while (!(ADC1->SR & ADC_SR_EOC));
     * uint16_t adc_val = ADC1->DR;
     * float tensao_pino_mv = (adc_val * ADC_VREF_MV) / ((1 << ADC_RESOLUCAO_BITS) - 1);
     * tensao_atual_mv = (uint16_t)(tensao_pino_mv * BATERIA_DIVISOR_RESISTIVO_FATOR);
     */

    /* Simulação temporária mantendo o valor nominal */
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
