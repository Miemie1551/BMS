#ifndef __BMS_GLOBAL_H
#define __BMS_GLOBAL_H

#include "main.h"

typedef struct
{
    uint16_t battery_soc; // 电池组剩余电量百分比，单位：%

    uint16_t cell_max_voltage;            // 单体最高电压，单位：mV
    uint16_t cell_min_voltage;            // 单体最低电压，单位：mV
    uint16_t cell_max_voltage_difference; // 单体最高与最低电压差，单位：mV
    uint16_t cell_avg_voltage;            // 单体平均电压，单位：mV

    uint16_t battery_voltage; // 电池组总电压，单位：mV
    int16_t battery_current;  // 电池组电流，单位：mA
    int16_t temperature;      // 电池组温度，单位：℃

    uint16_t cell_voltages_array[9]; // 9个单体电压数组，单位：mV
} BMS_Data_t;

extern BMS_Data_t g_bms_data;

#endif /* __BMS_GLOBAL_H */
