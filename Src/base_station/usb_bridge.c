/**
 * @file usb_bridge.c
 * @brief Ponte USB CDC com o PC.
 */

#include "usb_bridge.h"

#include <string.h>

#include "config.h"
#include "hal_plataforma.h"
#include "usbd_cdc_if.h"
#include "usb_device.h"

#define USB_BRIDGE_TX_TIMEOUT_MS   100U

static uint8_t rx_buffer[TAMANHO_BUFFER_USB_RX];
static uint16_t rx_head;
static uint16_t rx_tail;
static bool inicializado;

static uint16_t ring_next(uint16_t value)
{
    value++;
    return value >= TAMANHO_BUFFER_USB_RX ? 0U : value;
}

static bool ring_empty(void)
{
    return rx_head == rx_tail;
}

static bool ring_full(void)
{
    return ring_next(rx_head) == rx_tail;
}

static void ring_push(uint8_t byte)
{
    if (ring_full()) {
        rx_tail = ring_next(rx_tail);
    }
    rx_buffer[rx_head] = byte;
    rx_head = ring_next(rx_head);
}

static bool ring_pop(uint8_t *byte)
{
    if (byte == NULL || ring_empty()) return false;
    *byte = rx_buffer[rx_tail];
    rx_tail = ring_next(rx_tail);
    return true;
}

static void poll_cdc(void)
{
    uint8_t temp[64];
    while (CDC_Receive_Available_FS()) {
        uint16_t lidos = CDC_Read_FS(temp, (uint16_t)sizeof(temp));
        for (uint16_t i = 0U; i < lidos; ++i) {
            ring_push(temp[i]);
        }
        if (lidos == 0U) break;
    }
}

status_t usb_bridge_inicializar(void)
{
    rx_head = 0U;
    rx_tail = 0U;
    MX_USB_DEVICE_Init();
    inicializado = true;
    return STATUS_OK;
}

status_t usb_bridge_enviar(const uint8_t *dados, uint16_t tamanho)
{
    uint32_t inicio;
    if (!inicializado || dados == NULL || tamanho == 0U) return STATUS_ERRO_USB;

    inicio = plataforma_obter_tick_ms();
    do {
        uint8_t resultado = CDC_Transmit_FS((uint8_t *)dados, tamanho);
        if (resultado == USBD_OK) return STATUS_OK;
        if (resultado != USBD_BUSY) return STATUS_ERRO_USB;
    } while ((plataforma_obter_tick_ms() - inicio) < USB_BRIDGE_TX_TIMEOUT_MS);

    return STATUS_ERRO_TIMEOUT;
}

status_t usb_bridge_enviar_pacote_telemetria(const telemetry_packet_t *pacote)
{
    if (pacote == NULL) return STATUS_ERRO_GENERICO;
    return usb_bridge_enviar((const uint8_t *)pacote, (uint16_t)sizeof(*pacote));
}

status_t usb_bridge_enviar_status(uint8_t rssi, uint8_t snr, uint32_t pacotes_recebidos)
{
    uint8_t pacote[8] = {
        PROTOCOL_HEADER_BYTE,
        0xF0U,
        rssi,
        snr,
        (uint8_t)(pacotes_recebidos & 0xFFU),
        (uint8_t)((pacotes_recebidos >> 8) & 0xFFU),
        (uint8_t)((pacotes_recebidos >> 16) & 0xFFU),
        (uint8_t)((pacotes_recebidos >> 24) & 0xFFU)
    };
    return usb_bridge_enviar(pacote, (uint16_t)sizeof(pacote));
}

bool usb_bridge_dados_disponiveis(void)
{
    if (!inicializado) return false;
    poll_cdc();
    return !ring_empty();
}

status_t usb_bridge_receber(uint8_t *buffer, uint16_t *tamanho)
{
    uint16_t count = 0U;
    if (!inicializado || buffer == NULL || tamanho == NULL) return STATUS_ERRO_USB;

    poll_cdc();
    while (count < TAMANHO_BUFFER_USB_RX && ring_pop(&buffer[count])) {
        count++;
    }

    *tamanho = count;
    return count > 0U ? STATUS_OK : STATUS_ERRO_TIMEOUT;
}

bool usb_bridge_esta_conectado(void)
{
    return inicializado;
}
