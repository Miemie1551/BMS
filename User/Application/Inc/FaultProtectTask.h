#ifndef __FAULTPROTECTTASK_H
#define __FAULTPROTECTTASK_H

#include "stm32f1xx_hal.h"

// 系统保护参数结构体
typedef struct
{
    uint16_t OV_Relieve; // 充电过压解除保护阈值(mV)
    uint16_t UV_Relieve; // 放电欠压解除保护阈值(mV)

    uint16_t OCC_Protect;     // 充电过流保护阈值(mA)
    uint16_t OCC_RelieveTime; // 充电过流解除保护时间(s)
    uint16_t OCC_Delay;       // 充电过流保护延迟时间(s)

    uint16_t OCD_RelieveTime; // 放电过流解除保护时间(s)
    uint16_t SCD_RelieveTime; // 放电短路解除保护时间(s)

    int16_t LTC_Protect; // 充电低温保护阈值(℃)
    int16_t LTC_Relieve; // 充电低温解除保护阈值(℃)
    int16_t LTD_Protect; // 放电低温保护阈值(℃)
    int16_t LTD_Relieve; // 放电低温解除保护阈值(℃)

    int16_t OTC_Protect; // 充电过温保护阈值(℃)
    int16_t OTC_Relieve; // 充电过温解除保护阈值(℃)
    int16_t OTD_Protect; // 放电过温保护阈值(℃)
    int16_t OTD_Relieve; // 放电过温解除保护阈值(℃)
} BMS_ProtectParams_t;

// 系统保护报警枚举体
typedef enum
{
    FLAG_ALERT_NONE = 0x0000,     // 无报警触发
    FlAG_ALERT_OV = 0X0001,       // 充电过压保护触发位
    FlAG_ALERT_UV = 0X0002,       // 放电欠压保护触发位
    FlAG_ALERT_OCC = 0X0004,      // 充电过流保护触发位
    FlAG_ALERT_OCD = 0X0008,      // 放电过流保护触发位
    FlAG_ALERT_SCD = 0X0010,      // 放电短路保护触发位
    FlAG_ALERT_OTC = 0X0020,      // 充电过温保护触发位
    FlAG_ALERT_OTD = 0X0040,      // 放电过温保护触发位
    FlAG_ALERT_LTC = 0X0080,      // 充电低温保护触发位
    FlAG_ALERT_LTD = 0X0100,      // 放电低温保护触发位
    FlAG_ALERT_AFE_COMM = 0X0200, // AFE通信故障触发位
} BMS_ProtectAlert_t;

// 保护任务状态
typedef enum
{
    PROTECT_STATE_MONITOR,      // 监控状态监控保护状态
    PROTECT_STATE_RELIEVE_WAIT, // 等待解除保护状态
    PROTECT_STATE_RELIEVE,      // 解除保护状态
} BMS_ProtectState_t;

extern BMS_ProtectAlert_t bms_protect_alert;

void FaultProtectTask(void *argument);

#endif /* __FAULTPROTECTTASK_H */
