#ifndef USBD_DESC_H
#define USBD_DESC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_def.h"

#define DEVICE_ID1          (0x1FFF7A10UL)
#define DEVICE_ID2          (0x1FFF7A14UL)
#define DEVICE_ID3          (0x1FFF7A18UL)

#define USB_SIZ_STRING_SERIAL 0x1AU

extern USBD_DescriptorsTypeDef FS_Desc;

#ifdef __cplusplus
}
#endif

#endif /* USBD_DESC_H */
