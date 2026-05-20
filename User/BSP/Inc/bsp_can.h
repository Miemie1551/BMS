#ifndef __BSP_CAN_H
#define __BSP_CAN_H

#include "stm32f1xx_hal.h"

extern void BSP_CAN_Init(void);
extern void BSP_CAN_SendStdMsg(uint32_t std_id, uint8_t *data, uint8_t len);

#endif /* __BSP_CAN_H */
