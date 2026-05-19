#ifndef __DATAACQTASK_H
#define __DATAACQTASK_H

#include "stm32f1xx_hal.h"
#include "BMSInfo.h"

// 采样数据结构定义
typedef struct
{
    uint16_t cell_voltages[BMS_NUM_CELLS]; // 电芯单体电压（mV）
    uint16_t battery_voltage;              // 电池包总电压（mV）
    int16_t current;                       // 电池包总电流（mA）
    int16_t temperature;                   // 温度（0.1℃）

    uint16_t max_voltage;      // 最大电压（mV）
    uint16_t min_voltage;      // 最小电压（mV）
    uint16_t avg_voltage;      // 平均电压（mV）
    uint16_t max_voltage_diff; // 最大压差（mV）
} BMS_DataAcq_t;

extern BMS_DataAcq_t bms_data_acq;

void DataAcqTask(void *argument);

#endif /* __DATAACQTASK_H */
