#ifndef __BMSINFO_H
#define __BMSINFO_H

#include "main.h"
#include "cmsis_os.h"

// 单体电池数量
#define BMS_NUM_CELLS 9

// 电压阈值，单位：mV
#define BMS_CELL_OV_THRESHOLD 4200 // 单体过压
#define BMS_CELL_UV_THRESHOLD 3200 // 单体欠压

// 电池包电压极值，单位：mV
#define BMS_PACK_MAX_VOLTAGE (BMS_CELL_OV_THRESHOLD * BMS_NUM_CELLS) // 电池包最大电压
#define BMS_PACK_MIN_VOLTAGE (BMS_CELL_UV_THRESHOLD * BMS_NUM_CELLS) // 电池包最小电压

// 电流阈值，单位：mA
#define BMS_OCD_THRESHOLD 1000 // 放电过流
#define BMS_SCD_THRESHOLD 2000 // 放电短路

// 温度阈值，单位：0.1℃
#define BMS_OVERTEMP_THRESHOLD 60   // 过温
#define BMS_UNDERTEMP_THRESHOLD -20 // 欠温

// 均衡参数，单位：mV
#define BMS_BALANCE_VOLTAGE_THRESHOLD 50 // 平衡电压阈值

// 状态转换电流阈值，单位：mA
#define BMS_CHARGING_THRESHOLD 50     // 充电电流阈值
#define BMS_DISCHARGING_THRESHOLD -50 // 放电电流阈值

// BMS状态枚举
typedef enum
{
    BMS_STATE_STANDBY = 0,
    BMS_STATE_CHARGING = 1,
    BMS_STATE_DISCHARGING = 2,
    BMS_STATE_FAULT = 3
} BMS_State_t;

// 故障类型枚举
typedef enum
{
    FAULT_NONE = 0x0000, // 无故障

    FAULT_CELL_OV = 0x0001, // 单体过压
    FAULT_CELL_UV = 0x0002, // 单体欠压
    FAULT_PACK_OV = 0x0004, // 总压过压
    FAULT_PACK_UV = 0x0008, // 总压欠压

    FAULT_CHG_OC = 0x0010,        // 充电过流
    FAULT_DSG_OC = 0x0020,        // 放电过流
    FAULT_SHORT_CIRCUIT = 0x0040, // 短路
    FAULT_OVERTEMP = 0x0080,      // 过温

    FAULT_UNDERTEMP = 0x01000, // 欠温
    FAULT_AFE_COMM = 0x02000,  // AFE通信故障
} BMS_Fault_t;

// BMS数据结构定义
typedef struct
{
    // 电芯数据
    struct
    {
        uint16_t voltages[BMS_NUM_CELLS]; // 单体电压（mV）
        uint16_t max_voltage;             // 最高单体电压（mV）
        uint16_t min_voltage;             // 最低单体电压（mV）
        uint16_t avg_voltage;             // 平均电压（mV）
        uint16_t voltage_dif;             // 压差（mV）
    } cell_data;

    // 电池包数据
    struct
    {
        uint16_t voltage; // 总电压（mV）
        int16_t current;  // 总电流（mA）
        // int32_t power;    // 功率（mW）
        int16_t temperature; // 温度（0.1℃）
    } pack_data;

    // SOC/SOH
    uint8_t soc; // 剩余电量（%）
    // uint8_t soh;                 // 健康状态（%）
    // uint32_t capacity_remaining; // 剩余容量（mAh）
    // uint32_t capacity_full;      // 满容量（mAh）

    BMS_Fault_t fault; // 故障类型
    BMS_State_t state; // 状态
} BMS_Info_t;

void BMS_Info_Init(void);
void BMS_CopyBMSInfo(BMS_Info_t *dest);
void BMS_GetBMSInfoPtr(BMS_Info_t **dest);
void BMS_AcquireBMSInfoMutex(void);
void BMS_ReleaseBMSInfoMutex(void);

#endif /* __BMSINFO_H */
