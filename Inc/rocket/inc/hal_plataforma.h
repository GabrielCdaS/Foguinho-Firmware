/**
 * @file hal_plataforma.h
 * @brief Abstração de hardware para o temporizador do sistema (SysTick).
 *
 * Implementação bare-metal utilizando os registradores do núcleo Cortex-M4
 * para evitar dependência de bibliotecas HAL externas.
 */

#ifndef HAL_PLATAFORMA_H
#define HAL_PLATAFORMA_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

#ifndef HOST_TEST
/* ============================================================================
 * Registradores do SysTick (Cortex-M4)
 * ========================================================================= */
#define SYSTICK_BASE        (0xE000E010UL)
#define SYSTICK_CTRL        (*(volatile uint32_t *)(SYSTICK_BASE + 0x00))
#define SYSTICK_LOAD        (*(volatile uint32_t *)(SYSTICK_BASE + 0x04))
#define SYSTICK_VAL         (*(volatile uint32_t *)(SYSTICK_BASE + 0x08))

#define SYSTICK_CTRL_ENABLE_MSK     (1UL << 0)
#define SYSTICK_CTRL_TICKINT_MSK    (1UL << 1)
#define SYSTICK_CTRL_CLKSOURCE_MSK  (1UL << 2)
#endif

/* Variável global de timestamp incrementada na ISR do SysTick */
extern volatile uint32_t ms_ticks;
extern volatile bool tick_pendente;

/**
 * @brief Inicializa o SysTick para gerar uma interrupção a cada milissegundo.
 */
static inline void plataforma_inicializar_systick(void) {
#ifndef HOST_TEST
    /* O SysTick conta de LOAD até 0, portanto LOAD = (Clock / Frequência) - 1 */
    /* Para 1 kHz (1 ms) com CLOCK_SISTEMA_HZ = 100MHz, LOAD = 100000 - 1 */
    uint32_t ticks = CLOCK_SISTEMA_HZ / 1000;
    SYSTICK_LOAD = (ticks - 1);
    SYSTICK_VAL = 0;
    SYSTICK_CTRL = SYSTICK_CTRL_CLKSOURCE_MSK | SYSTICK_CTRL_TICKINT_MSK | SYSTICK_CTRL_ENABLE_MSK;
#endif
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
    uint32_t inicio = ms_ticks;
    while ((ms_ticks - inicio) < ms) {
        /* Aguarda */
    }
}

#endif /* HAL_PLATAFORMA_H */
