#ifndef __STATECONTROLTASK_H
#define __STATECONTROLTASK_H

#include "stm32f1xx_hal.h"
#include "BMSInfo.h"

// BMS状态枚举
typedef enum
{
    BMS_STATE_STANDBY = 0,
    BMS_STATE_CHARGING = 1,
    BMS_STATE_DISCHARGING = 2,
    BMS_STATE_FAULT = 3
} BMS_SysState_t;

typedef struct
{
    uint8_t state_CHG;
    uint8_t state_DSG;
} BMS_FETState_t;

extern BMS_SysState_t bms_sys_state;
extern BMS_FETState_t bms_fet_state;

void StateControlTask(void *argument);
void BMS_ControlCharge(uint8_t enable);
void BMS_ControlDischarge(uint8_t enable);
void BMS_UpdateState_CHGAndDSG(uint8_t CHG_new_state, uint8_t DSG_new_state);

#endif /* __STATECONTROLTASK_H */
