/**
 * @file hal_plataforma.h
 * @brief Abstração mínima de tempo para o firmware da estação base.
 */

#ifndef BASE_STATION_HAL_PLATAFORMA_H
#define BASE_STATION_HAL_PLATAFORMA_H

#include <stdbool.h>
#include <stdint.h>
#include "config.h"

#ifndef HOST_TEST
#include "stm32f4xx_hal.h"
#endif

extern volatile uint32_t ms_ticks;
extern volatile bool tick_pendente;

static inline void plataforma_inicializar_systick(void)
{
    /* HAL_Init e SystemClock_Config configuram o SysTick em 1 kHz. */
}

static inline uint32_t plataforma_obter_tick_ms(void)
{
#ifndef HOST_TEST
    return HAL_GetTick();
#else
    return ms_ticks;
#endif
}

static inline void plataforma_delay_ms(uint32_t ms)
{
#ifndef HOST_TEST
    HAL_Delay(ms);
#else
    uint32_t inicio = ms_ticks;
    while ((ms_ticks - inicio) < ms) {
    }
#endif
}

#endif /* BASE_STATION_HAL_PLATAFORMA_H */
