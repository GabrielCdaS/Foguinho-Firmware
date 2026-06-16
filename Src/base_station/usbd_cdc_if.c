#include "usbd_cdc_if.h"

#include <string.h>

#include "usb_device.h"

#define APP_RX_DATA_SIZE  256U
#define APP_TX_DATA_SIZE  256U

static uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];
static uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];
static uint8_t cdc_ring[APP_RX_DATA_SIZE];
static volatile uint16_t cdc_head;
static volatile uint16_t cdc_tail;

static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t *pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t *pbuf, uint32_t *Len);

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS = {
    CDC_Init_FS,
    CDC_DeInit_FS,
    CDC_Control_FS,
    CDC_Receive_FS
};

static uint16_t ring_next(uint16_t value)
{
    value++;
    return value >= APP_RX_DATA_SIZE ? 0U : value;
}

static void ring_push(uint8_t byte)
{
    uint16_t next = ring_next(cdc_head);
    if (next == cdc_tail) {
        cdc_tail = ring_next(cdc_tail);
    }
    cdc_ring[cdc_head] = byte;
    cdc_head = next;
}

static bool ring_pop(uint8_t *byte)
{
    if (byte == NULL || cdc_head == cdc_tail) return false;
    *byte = cdc_ring[cdc_tail];
    cdc_tail = ring_next(cdc_tail);
    return true;
}

static int8_t CDC_Init_FS(void)
{
    cdc_head = 0U;
    cdc_tail = 0U;
    USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0U);
    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
    return USBD_OK;
}

static int8_t CDC_DeInit_FS(void)
{
    return USBD_OK;
}

static int8_t CDC_Control_FS(uint8_t cmd, uint8_t *pbuf, uint16_t length)
{
    (void)cmd;
    (void)pbuf;
    (void)length;
    return USBD_OK;
}

static int8_t CDC_Receive_FS(uint8_t *pbuf, uint32_t *Len)
{
    if (pbuf != NULL && Len != NULL) {
        for (uint32_t i = 0U; i < *Len; ++i) {
            ring_push(pbuf[i]);
        }
    }
    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
    (void)USBD_CDC_ReceivePacket(&hUsbDeviceFS);
    return USBD_OK;
}

uint8_t CDC_Transmit_FS(uint8_t *Buf, uint16_t Len)
{
    USBD_CDC_HandleTypeDef *hcdc;
    if (Buf == NULL || Len == 0U || hUsbDeviceFS.pClassData == NULL) {
        return USBD_FAIL;
    }

    hcdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData;
    if (hcdc->TxState != 0U) {
        return USBD_BUSY;
    }

    if (Len > APP_TX_DATA_SIZE) {
        Len = APP_TX_DATA_SIZE;
    }
    memcpy(UserTxBufferFS, Buf, Len);
    USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, Len);
    return USBD_CDC_TransmitPacket(&hUsbDeviceFS);
}

bool CDC_Receive_Available_FS(void)
{
    return cdc_head != cdc_tail;
}

uint16_t CDC_Read_FS(uint8_t *buffer, uint16_t max_len)
{
    uint16_t count = 0U;
    if (buffer == NULL) return 0U;
    while (count < max_len && ring_pop(&buffer[count])) {
        count++;
    }
    return count;
}
