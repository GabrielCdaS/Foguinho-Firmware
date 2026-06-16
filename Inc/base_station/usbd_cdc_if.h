#ifndef USBD_CDC_IF_H
#define USBD_CDC_IF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "usbd_cdc.h"

extern USBD_CDC_ItfTypeDef USBD_Interface_fops_FS;

uint8_t CDC_Transmit_FS(uint8_t *Buf, uint16_t Len);
bool CDC_Receive_Available_FS(void);
uint16_t CDC_Read_FS(uint8_t *buffer, uint16_t max_len);

#ifdef __cplusplus
}
#endif

#endif /* USBD_CDC_IF_H */
