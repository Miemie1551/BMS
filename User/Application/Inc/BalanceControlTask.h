#ifndef __BananceControlTask_H
#define __BananceControlTask_H

#include "stm32f1xx_hal.h"

// 电芯均衡控制状态
typedef enum
{
    BMS_BALANCE_STATUS_IDLE = 0,
    BMS_BALANCE_STATUS_RUNNING,
} BMS_BALANCE_STATUS;

typedef struct
{
    uint8_t balance_cell_index;        // 正在均衡的电芯索引
    BMS_BALANCE_STATUS balance_status; // 电芯均衡状态
} BMS_BalanceInfo;

extern BMS_BalanceInfo bms_balance_info;

void BalanceControlTask(void *argument);

#endif /* __BalanceControlTask_H */
