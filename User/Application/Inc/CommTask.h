#ifndef __COMMTASK_H
#define __COMMTASK_H

#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

typedef struct
{
    uint32_t id;
    uint8_t data[8];
    uint8_t dlc;
} CAN_RxMsg_t;

extern osMessageQueueId_t canRxMQHandle;

void CommTask(void *argument);

#endif /* __COMMTASK_H */
