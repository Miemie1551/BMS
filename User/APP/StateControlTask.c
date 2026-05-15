#include "StateControlTask.h"
#include "cmsis_os.h"
#include "usart.h"

#include "bms_global.h"
#include "bsp_bq76940.h"

#define LOG_E(fmt, ...) printf("[ERROR] [%s:%d]" fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_D(fmt, ...) printf("[DEBUG] " fmt, ##__VA_ARGS__)

#define STATE_CHECK_PERIOD_MS 1000

static BMS_Info_t info;
static BMS_Info_t *info_ptr = NULL;

static void BMS_StandbyStateHandler(void);
static void BMS_ChargingStateHandler(void);
static void BMS_DischargingStateHandler(void);
static void BMS_FaultStateHandler(void);

static void BMS_EnterChargingMode(void);
static void BMS_EnterDischargingMode(void);
static void BMS_EnterStandbyMode(void);
static void BMS_EnterFaultMode(void);

void StateControlTask(void *argument)
{
    for (;;)
    {
        // 获取当前电池信息
        BMS_AcquireBMSInfoMutex(); // 获取互斥锁
        BMS_CopyBMSInfo(&info);    // 复制最新BMS信息
        BMS_ReleaseBMSInfoMutex(); // 释放互斥锁

        // 状态机转换
        switch (info.state)
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

        // 更新状态
        BMS_AcquireBMSInfoMutex();    // 获取互斥锁
        BMS_GetBMSInfoPtr(info_ptr);  // 获取最新BMS信息指针
        info_ptr->state = info.state; // 更新状态
        BMS_ReleaseBMSInfoMutex();    // 释放互斥锁

        osDelay(STATE_CHECK_PERIOD_MS);
    }
}

/**
 * @brief 待机状态处理逻辑
 * @retval None
 */
static void BMS_StandbyStateHandler(void)
{
    // 检测故障
    if (info.fault != FAULT_NONE)
    {
        info.state = BMS_STATE_FAULT;
        BMS_EnterFaultMode();
    }
    // 检测充电或放电请求
    else if (info.pack_data.current > BMS_CHARGING_THRESHOLD)
    {
        info.state = BMS_STATE_CHARGING;
    }
    else if (info.pack_data.current < BMS_DISCHARGING_THRESHOLD)
    {
        info.state = BMS_STATE_DISCHARGING;
    }
}

static void BMS_ChargingStateHandler(void)
{
    // 检测故障
    if (info.fault != FAULT_NONE)
    {
        info.state = BMS_STATE_FAULT;
        BMS_EnterFaultMode();
    }
    // 充电完成或停止放电
    else if (info.soc >= 100 || info.pack_data.current < BMS_CHARGING_THRESHOLD)
    {
        info.state = BMS_STATE_STANDBY;
        // BMS_EnterStandbyMode();
    }
}

static void BMS_DischargingStateHandler(void)
{
    // 检测故障
    if (info.fault != FAULT_NONE)
    {
        info.state = BMS_STATE_FAULT;
        BMS_EnterFaultMode();
    }
    // 放电完成或停止充电
    else if (info.soc <= 0 || info.pack_data.current > BMS_DISCHARGING_THRESHOLD)
    {
        info.state = BMS_STATE_STANDBY;
        // BMS_EnterStandbyMode();
    }
}

static void BMS_FaultStateHandler(void)
{
    // 故障状态保持，等待故障清除
    if (info.fault == FAULT_NONE)
    {
        info.state = BMS_STATE_STANDBY;
        BMS_EnterStandbyMode();
    }
}

static void BMS_EnterChargingMode(void)
{
}

static void BMS_EnterDischargingMode(void)
{
}

static void BMS_EnterStandbyMode(void)
{
    // 打开充电放电回路
    BQ76940_EnableCharging();
    BQ76940_EnableDischarging();
    LOG_D("Turn on the charging and discharging circuitry\r\n");
}

static void BMS_EnterFaultMode(void)
{
    // 关闭充电放电回路
    BQ76940_DisableCharging();
    BQ76940_DisableDischarging();
    LOG_D("Turn off the charging and discharging circuitry\r\n");
}
