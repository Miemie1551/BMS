#include "StateControlTask.h"
#include "cmsis_os.h"

#include "DataAcqTask.h"
#include "FaultProtectTask.h"
#include "BatteryGaugeTask.h"

#include "bsp_usart.h"
#include "dev_bq76940.h"

#define LOG_ENABLE 1
#if LOG_ENABLE
#define LOG_E(fmt, ...) Printf("[ERROR] [%s:%d]" fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_D(fmt, ...) Printf("[DEBUG] " fmt, ##__VA_ARGS__)
#else
#define LOG_E(...)
#define LOG_D(...)
#endif

#define STATE_CHECK_PERIOD_MS 1000

// 状态转换电流阈值
#define BMS_CHARGING_THRESHOLD 50     // 充电电流阈值（mA）
#define BMS_DISCHARGING_THRESHOLD -50 // 放电电流阈值（mA）

BMS_State_t bms_state = BMS_STATE_STANDBY;
BMS_FETState_t bms_fet_state = {1, 1};

static void BMS_StandbyStateHandler(void);
static void BMS_ChargingStateHandler(void);
static void BMS_DischargingStateHandler(void);
static void BMS_FaultStateHandler(void);

void StateControlTask(void *argument)
{
    for (;;)
    {
        // 检测故障
        if (bms_protect_alert != FLAG_ALERT_NONE)
        {
            bms_state = BMS_STATE_FAULT;
        }

        // 状态机转换
        switch (bms_state)
        {
        case BMS_STATE_STANDBY:
            BMS_StandbyStateHandler();
            break;
        case BMS_STATE_CHARGING:
            BMS_ChargingStateHandler();
            break;
        case BMS_STATE_DISCHARGING:
            BMS_DischargingStateHandler();
            break;
        case BMS_STATE_FAULT:
            BMS_FaultStateHandler();
            break;
        }

        osDelay(STATE_CHECK_PERIOD_MS);
    }
}

/**
 * @brief 待机状态处理逻辑
 * @retval None
 */
static void BMS_StandbyStateHandler(void)
{
    // 充电状态判断：充电电流超过预设阈值
    if (bms_data_acq.current > BMS_CHARGING_THRESHOLD)
    {
        bms_state = BMS_STATE_CHARGING;
    }
    // 放电状态判断：放电电流低于预设阈值
    else if (bms_data_acq.current < BMS_DISCHARGING_THRESHOLD)
    {
        bms_state = BMS_STATE_DISCHARGING;
    }

    static uint8_t charge_count = 0;    // 充电请求计数器
    static uint8_t discharge_count = 0; // 放电请求计数器

    /* 根据充放电开关状态和电池SOC阈值判断是否需要充电或放电 */
    if (bms_fet_state.state_CHG == 0 && bms_battery_gauge_data.soc < BMS_START_CHARGING_SOC)
    {
        charge_count++;
        if (charge_count >= 3)
        {
            charge_count = 0;
            BMS_ControlCharge(1);
        }
    }
    if (bms_fet_state.state_DSG == 0 && bms_battery_gauge_data.soc > BMS_START_DISCHARGING_SOC)
    {
        discharge_count++;
        if (discharge_count >= 3)
        {
            discharge_count = 0;
            BMS_ControlDischarge(1);
        }
    }
}

static void BMS_ChargingStateHandler(void)
{
    static uint8_t charge_completed_count = 0; // 充电完成计数器

    // 停止充电判断：电流低于阈值
    if (bms_data_acq.current < BMS_CHARGING_THRESHOLD)
    {
        bms_state = BMS_STATE_STANDBY;
    }
    // 充电完成判断：电池SOC超过满电阈值
    else if (bms_battery_gauge_data.soc >= BMS_STOP_CHARGING_SOC)
    {
        charge_completed_count++;
        if (charge_completed_count >= 3)
        {
            charge_completed_count = 0;
            bms_state = BMS_STATE_STANDBY;
            BMS_ControlCharge(0);
        }
    }
}

static void BMS_DischargingStateHandler(void)
{
    static uint8_t discharge_completed_count = 0; // 放电完成计数器

    // 停止放电判断：电流低于阈值
    if (bms_data_acq.current > BMS_DISCHARGING_THRESHOLD)
    {
        bms_state = BMS_STATE_STANDBY;
    }
    // 放电完成判断：电池SOC低于空电阈值
    else if (bms_battery_gauge_data.soc <= BMS_STOP_DISCHARGING_SOC)
    {
        discharge_completed_count++;
        if (discharge_completed_count >= 3)
        {
            discharge_completed_count = 0;
            bms_state = BMS_STATE_STANDBY;
            BMS_ControlDischarge(0);
        }
    }
}

static void BMS_FaultStateHandler(void)
{
    // 故障状态保持，等待故障清除
    if (bms_protect_alert == FLAG_ALERT_NONE)
    {
        bms_state = BMS_STATE_STANDBY;
    }
}

void BMS_ControlCharge(uint8_t enable)
{
    bms_fet_state.state_CHG = enable;
    if (enable)
    {
        BQ76940_EnableCharging();
    }
    else
    {
        BQ76940_DisableCharging();
    }
}

void BMS_ControlDischarge(uint8_t enable)
{
    bms_fet_state.state_DSG = enable;
    if (enable)
    {
        BQ76940_EnableDischarging();
    }
    else
    {
        BQ76940_DisableDischarging();
    }
}

void BMS_UpdateState_CHGAndDSG(uint8_t CHG_new_state, uint8_t DSG_new_state)
{
    bms_fet_state.state_CHG = CHG_new_state;
    bms_fet_state.state_DSG = DSG_new_state;
}
