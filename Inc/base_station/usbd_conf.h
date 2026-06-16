#ifndef USBD_CONF_H
#define USBD_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f4xx_hal.h"

#define USBD_MAX_NUM_INTERFACES     1U
#define USBD_MAX_NUM_CONFIGURATION  1U
#define USBD_MAX_STR_DESC_SIZ       0x100U
#define USBD_SUPPORT_USER_STRING    0U
#define USBD_SELF_POWERED           0U
#define USBD_DEBUG_LEVEL            0U
#define USBD_LPM_ENABLED            0U

void *USBD_static_malloc(uint32_t size);
void USBD_static_free(void *p);

#define USBD_malloc         (uint32_t *)USBD_static_malloc
#define USBD_free           USBD_static_free
#define USBD_memset         memset
#define USBD_memcpy         memcpy
#define USBD_Delay          HAL_Delay

#if (USBD_DEBUG_LEVEL > 0)
#define USBD_UsrLog(...)    printf(__VA_ARGS__); printf("\n")
#else
#define USBD_UsrLog(...)
#endif

#define USBD_ErrLog(...)    do { } while (0)
#define USBD_DbgLog(...)    do { } while (0)

#ifdef __cplusplus
}
#endif

#endif /* USBD_CONF_H */
