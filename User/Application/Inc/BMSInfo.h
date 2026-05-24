#ifndef __BMSINFO_H
#define __BMSINFO_H

#include "stm32f1xx_hal.h"

// 单体电池数量
#define BMS_NUM_CELLS 9

// 电压阈值，单位：mV
#define BMS_OV_PROTECT 4200 // 单体过压保护阈值 OV范围：3.15~4.70V  3136~4674mV
#define BMS_OV_RELIEVE 4180 // 单体过压解除保护阈值
#define BMS_OV_DELAY 2      // 单体过压保护延迟时间(s)
#define BMS_UV_PROTECT 3100 // 单体欠压保护阈值 UV范围：1.58~3.10V  1589~3125mV
#define BMS_UV_RELIEVE 3150 // 单体欠压解除保护阈值
#define BMS_UV_DELAY 2      // 单体欠压保护延迟时间(s)

#define BMS_OCC_PROTECT 2000    // 充电过流保护阈值(mA)
#define BMS_OCC_DELAY 1         // 充电过流保护延迟时间(s)
#define BMS_OCC_RELIEVE_TIME 30 // 充电过流解除保护时间(s)

#define BMS_OCD_PROTECT 1000    // 放电过流保护阈值(mA)
#define BMS_OCD_RELIEVE_TIME 30 // 放电过流解除保护时间(s)
#define BMS_SCD_PROTECT 2000    // 放电短路保护阈值(mA)
#define BMS_SCD_RELIEVE_TIME 60 // 放电短路解除保护时间(s)

// 充放电温度阈值（放大10倍）
#define BMS_LTC_PROTECT -200 // 充电低温保护阈值(℃)
#define BMS_LTC_RELIEVE -150 // 充电低温解除保护阈值(℃)
#define BMS_LTD_PROTECT -200 // 放电低温保护阈值(℃)
#define BMS_LTD_RELIEVE -150 // 放电低温解除保护阈值(℃)

#define BMS_OTC_PROTECT 700 // 充电过温保护阈值(℃)
#define BMS_OTC_RELIEVE 600 // 充电过温解除保护阈值(℃)
#define BMS_OTD_PROTECT 700 // 放电过温保护阈值(℃)
#define BMS_OTD_RELIEVE 600 // 放电过温解除保护阈值(℃)

/* 充电放电SOC阈值(放大10倍，单位：%) */
#define BMS_STOP_CHARGING_SOC 100   // 停止充电SOC阈值
#define BMS_STOP_DISCHARGING_SOC 0  // 停止放电SOC阈值
#define BMS_START_CHARGING_SOC 99   // 开始充电SOC阈值
#define BMS_START_DISCHARGING_SOC 1 // 开始放电SOC阈值

//
typedef enum
{
    BMS_DISABLE = 0, // 禁用
    BMS_ENABLE = 1,  // 使能
} BMS_State_t;

// 用户配置
typedef struct
{
    BMS_State_t charge;    // 充电使能
    BMS_State_t discharge; // 放电使能
    BMS_State_t balance;   // 均衡使能
    BMS_State_t auto_mode; // 自动模式
} BMS_UserConfig_t;

extern BMS_UserConfig_t bms_user_config;

#endif /* __BMSINFO_H */
