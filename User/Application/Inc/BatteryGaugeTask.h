#ifndef __BATTERYGAGETASK_H
#define __BATTERYGAGETASK_H

#include "stm32f1xx_hal.h"

typedef struct
{
    uint16_t soc;              // 电池包SOC值（剩余电量百分比）
    uint16_t capacity_rated;  // 电池包额定容量（mAh）
    uint16_t capacity_real;   // 电池包实际容量（mAh）
    uint16_t capacity_remain; // 电池包剩余容量（mAh）
} BMS_BatteryGaugeData_t;

extern BMS_BatteryGaugeData_t bms_battery_gauge_data;

void BatteryGaugeTask(void *argument);

#endif /* __BATTERYGAGETASK_H */
