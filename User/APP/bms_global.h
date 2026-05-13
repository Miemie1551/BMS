#ifndef __BMS_GLOBAL_H
#define __BMS_GLOBAL_H

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
#define BMS_CHARGING_THRESHOLD 500   // 充电电流阈值
#define BMS_DISCHARGING_THRESHOLD 50 // 放电电流阈值

// BMS状态枚举
typedef enum
{
    BMS_STATE_STANDBY = 0,
    BMS_STATE_CHARGING = 1,
    BMS_STATE_DISCHARGING = 2,
    BMS_STATE_BALANCING = 3,
    BMS_STATE_FAULT = 4
} BMS_State_t;

// 故障类型枚举
typedef enum
{
    FAULT_NONE = 0x00,
    FAULT_CELL_OV = 0x01,       // 单体过压
    FAULT_CELL_UV = 0x02,       // 单体欠压
    FAULT_PACK_OV = 0x04,       // 总压过压
    FAULT_PACK_UV = 0x08,       // 总压欠压
    FAULT_OVERCURRENT = 0x10,   // 过流
    FAULT_SHORT_CIRCUIT = 0x20, // 短路
    FAULT_OVERTEMP = 0x40,      // 过温
    FAULT_UNDERTEMP = 0x80      // 欠温
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

extern BMS_Info_t g_bms_info;
extern osMutexId_t g_bms_info_mutex;

extern void BMS_Global_Init(void);
extern void BMS_Info_Read(BMS_Info_t *dest);
extern void BMS_Info_Write(const BMS_Info_t *src);

#endif /* __BMS_GLOBAL_H */
