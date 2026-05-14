#include "StateControlTask.h"
#include "cmsis_os.h"

#include "bms_global.h"

#include "bsp_bq76940.h"

#define STATE_CHECK_PERIOD_MS 100

static BMS_Info_t local_info;
static BMS_State_t current_state = BMS_STATE_STANDBY;

static void BMS_StandbyStateHandler(void);
static void BMS_ChargingStateHandler(void);
static void BMS_DischargingStateHandler(void);
static void BMS_FaultStateHandler(void);

static void BMS_EnterChargingMode(void);
static void BMS_EnterDischargingMode(void);
static void BMS_EnterStandbyMode(void);

void StateControlTask(void *argument)
{
    for (;;)
    {
        // 获取当前电池信息
        BMS_Info_Read(&local_info);

        // 状态机转换
        switch (current_state)
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
        default:
            break;
        }

        // 更新状态
        BMS_Info_Write(&local_info);

        osDelay(STATE_CHECK_PERIOD_MS);
    }
}

/**
 * @brief 待机状态处理逻辑
 * @retval None
 */
static void BMS_StandbyStateHandler(void)
{
    // 检测充电或放电请求
    if (local_info.pack_data.current > BMS_CHARGING_THRESHOLD)
    {
        current_state = BMS_STATE_CHARGING;
        BMS_EnterChargingMode();
    }
    else if (local_info.pack_data.current < BMS_DISCHARGING_THRESHOLD)
    {
        current_state = BMS_STATE_DISCHARGING;
        BMS_EnterDischargingMode();
    }
}

static void BMS_ChargingStateHandler(void)
{
    // 充电完成或故障退出
    if (local_info.soc >= 100 || local_info.fault != FAULT_NONE)
    {
        current_state = BMS_STATE_STANDBY;
        BMS_EnterStandbyMode();
    }
}

static void BMS_DischargingStateHandler(void)
{
    // 放电完成或故障退出
    if (local_info.soc <= 0 || local_info.fault != FAULT_NONE)
    {
        current_state = BMS_STATE_STANDBY;
        BMS_EnterStandbyMode();
    }
}

static void BMS_FaultStateHandler(void)
{
    // 故障状态保持，等待故障清除
    if (local_info.fault == FAULT_NONE)
    {
        current_state = BMS_STATE_STANDBY;
        BMS_EnterStandbyMode();
    }
}

static void BMS_EnterChargingMode(void)
{
    // 开启充电回路
    BQ76940_EnableCharging();
}

static void BMS_EnterDischargingMode(void)
{
    // 开启放电回路
    BQ76940_EnableDischarging();
}

static void BMS_EnterStandbyMode(void)
{
    // 关闭充电放电回路
    BQ76940_DisableCharging();
    BQ76940_DisableDischarging();
}
