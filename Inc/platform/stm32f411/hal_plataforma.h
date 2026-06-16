/**
 * @file hal_plataforma.h
 * @brief Abstração de hardware para o temporizador do sistema (SysTick).
 *
 * Implementação baseada no tick oficial da STM32Cube HAL.
 */

#ifndef HAL_PLATAFORMA_H
#define HAL_PLATAFORMA_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

#ifndef HOST_TEST
#include "stm32f4xx_hal.h"
#endif

/* Variável global de timestamp incrementada na ISR do SysTick */
extern volatile uint32_t ms_ticks;
extern volatile bool tick_pendente;

/**
 * @brief Inicializa o SysTick para gerar uma interrupção a cada milissegundo.
 */
static inline void plataforma_inicializar_systick(void) {
    /* HAL_Init e HAL_RCC_ClockConfig configuram o SysTick em 1 kHz. */
}

/**
 * @brief Retorna o tempo do sistema em milissegundos.
 */
static inline uint32_t plataforma_obter_tick_ms(void) {
    return ms_ticks;
}

/**
 * @brief Aguarda de forma bloqueante pelo tempo especificado em milissegundos.
 */
static inline void plataforma_delay_ms(uint32_t ms) {
#ifndef HOST_TEST
    HAL_Delay(ms);
#else
    uint32_t inicio = ms_ticks;
    while ((ms_ticks - inicio) < ms) {
    }
#endif
}

#endif /* HAL_PLATAFORMA_H */
