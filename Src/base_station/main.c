/**
 * @file main.c
 * @brief Firmware da estação base Foguinho para STM32F411CEU6 Black Pill.
 */

#include <stdbool.h>
#include <stdint.h>

#include "config.h"
#include "hal_plataforma.h"
#include "pinout.h"
#include "protocol.h"
#include "stm32f411_hw.h"
#include "sx1278.h"
#include "usb_bridge.h"

volatile uint32_t ms_ticks = 0U;
volatile bool tick_pendente = false;

static void base_indicar_erro(void)
{
#if HABILITAR_LED_STATUS
    while (1) {
        hw_gpio_write(PINO_LED_STATUS_PORTA, PINO_LED_STATUS_PINO, true);
        plataforma_delay_ms(120U);
        hw_gpio_write(PINO_LED_STATUS_PORTA, PINO_LED_STATUS_PINO, false);
        plataforma_delay_ms(120U);
    }
#else
    while (1) {
    }
#endif
}

static void base_inicializar(void)
{
    hw_clock_init();
    plataforma_inicializar_systick();

#if HABILITAR_LED_STATUS
    hw_gpio_output(PINO_LED_STATUS_PORTA, PINO_LED_STATUS_PINO, false);
#endif

    if (usb_bridge_inicializar() != STATUS_OK) {
        base_indicar_erro();
    }
    if (sx1278_inicializar() != STATUS_OK) {
        base_indicar_erro();
    }
    if (protocolo_base_inicializar() != STATUS_OK) {
        base_indicar_erro();
    }
}

int main(void)
{
    uint32_t ultimo_blink_ms = 0U;
    bool led = false;

    base_inicializar();

    while (1) {
        (void)sx1278_processar();
        (void)protocolo_base_processar_radio();
        (void)protocolo_base_processar_usb();

#if HABILITAR_LED_STATUS
        uint32_t agora = plataforma_obter_tick_ms();
        if ((agora - ultimo_blink_ms) >= INTERVALO_HEARTBEAT_MS) {
            ultimo_blink_ms = agora;
            led = !led;
            hw_gpio_write(PINO_LED_STATUS_PORTA, PINO_LED_STATUS_PINO, led);
        }
#endif
    }
}
