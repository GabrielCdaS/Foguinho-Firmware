/**
 * @file battery.h
 * @brief Monitoramento de bateria LiPo 2S via ADC.
 *
 * Fornece funções para leitura de tensão, cálculo de porcentagem
 * e detecção de tensão baixa/crítica da bateria.
 *
 * Bateria: LiPo 2S (7,4V nominal)
 * Entrada ADC: PA0 (ADC1_CH0) com divisor resistivo
 * Resolução: 12 bits, Vref = 3,3V
 */

#ifndef BATTERY_H
#define BATTERY_H

#include <stdint.h>
#include <stdbool.h>
#include "sensors.h"

/* ============================================================================
 * Parâmetros da Bateria LiPo 2S
 * ========================================================================= */

/** Tensão nominal da bateria em milivolts (2S = 7,4V) */
#define BATERIA_TENSAO_NOMINAL_MV       7400

/** Tensão mínima operacional em milivolts (abaixo disso: alerta) */
#define BATERIA_TENSAO_MINIMA_MV        6400

/** Tensão crítica em milivolts (abaixo disso: desligamento seguro) */
#define BATERIA_TENSAO_CRITICA_MV       6000

/* ============================================================================
 * Parâmetros do ADC e Divisor Resistivo
 * ========================================================================= */

/**
 * Fator do divisor resistivo.
 * O divisor reduz a tensão da bateria (até ~8,4V) para a faixa do ADC (0-3,3V).
 * Exemplo: com fator 3.0, 7,4V -> ~2,47V no ADC.
 * Tensão real = leitura_ADC_mV * BATERIA_DIVISOR_RESISTIVO_FATOR
 */
#define BATERIA_DIVISOR_RESISTIVO_FATOR 3.0f

/** Resolução do ADC em bits */
#define ADC_RESOLUCAO_BITS              12

/** Tensão de referência do ADC em milivolts */
#define ADC_VREF_MV                     3300

/* ============================================================================
 * Funções de Monitoramento de Bateria
 * ========================================================================= */

/**
 * @brief Inicializa o módulo de monitoramento de bateria.
 *
 * Configura o canal ADC1_CH0 (PA0) para leitura da tensão da bateria
 * através do divisor resistivo.
 *
 * @return STATUS_OK em caso de sucesso.
 */
status_t bateria_inicializar(void);

/**
 * @brief Lê a tensão atual da bateria em milivolts.
 *
 * Realiza a conversão ADC, aplica o fator do divisor resistivo e retorna
 * a tensão real estimada da bateria.
 *
 * @return Tensão da bateria em milivolts.
 */
uint16_t bateria_ler_tensao_mv(void);

/**
 * @brief Calcula a porcentagem estimada de carga da bateria.
 *
 * Utiliza uma aproximação linear entre a tensão mínima (0%) e a tensão
 * nominal (100%).
 *
 * @return Porcentagem de carga (0 a 100).
 */
uint8_t bateria_ler_porcentagem(void);

/**
 * @brief Verifica se a tensão da bateria está abaixo do limite mínimo.
 *
 * Retorna verdadeiro se a tensão atual for menor que BATERIA_TENSAO_MINIMA_MV.
 *
 * @return true se a tensão está baixa, false caso contrário.
 */
bool bateria_tensao_baixa(void);

#endif /* BATTERY_H */
