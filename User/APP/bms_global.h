#ifndef __BMS_GLOBAL_H
#define __BMS_GLOBAL_H

#include "main.h"

#define BMS_NUM_CELLS 9

#define BMS_SCD_THRESHOLD 2000 // 放电短路电流阈值，单位：mA
#define BMS_SCD_DELAY 10       // 短路保护触发延时，单位：ms
#define BMS_OCD_THRESHOLD 1000 // 过流电流阈值，单位：mA
#define BMS_OCD_DELAY 1000     // 过流保护触发延时，单位：ms

#define BMS_OV_THRESHOLD 4250 // 过压阈值，单位：mV
#define BMS_OV_DELAY 1000     // 过压保护触发延时，单位：ms
#define BMS_UV_THRESHOLD 3200 // 欠压阈值，单位：mV
#define BMS_UV_DELAY 1000     // 欠压保护触发延时，单位：ms

#define BMS_OT_THRESHOLD 80  // 过温阈值，单位：℃
#define BMS_OT_DELAY 1000    // 过温保护触发延时，单位：ms
#define BMS_UT_THRESHOLD -20 // 欠温阈值，单位：℃
#define BMS_UT_DELAY 1000    // 欠温保护触发延时，单位：ms

#define BMS_BALANCE_VOLTAGE_THRESHOLD 4100          // 平衡电压阈值，单位：mV
#define BMS_BALANCE_VOLTAGE_DIFFERENCE_THRESHOLD 50 // 平衡电压差阈值，单位：mV

#define BMS_CELL_VOLTAGE_MAX 4200 // 单体电池最高电压，单位：mV
#define BMS_CELL_VOLTAGE_MIN 3000 // 单体电池最低电压，单位：mV

#define BMS_BATTERY_VOLTAGE_MAX (BMS_CELL_VOLTAGE_MAX * BMS_NUM_CELLS) // 电池组最高电压，单位：mV
#define BMS_BATTERY_VOLTAGE_MIN (BMS_CELL_VOLTAGE_MIN * BMS_NUM_CELLS) // 电池组最低电压，单位：mV
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

    uint16_t cell_voltages_array[BMS_NUM_CELLS]; // 单体电池电压数组，单位：mV
} BMS_Data_t;

extern BMS_Data_t g_bms_data;

#endif /* __BMS_GLOBAL_H */
