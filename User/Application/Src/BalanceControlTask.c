#include "BalanceControlTask.h"
#include "cmsis_os.h"

#include "BMSInfo.h"
#include "DataAcqTask.h"
#include "StateControlTask.h"

#include "dev_bq76940.h"
#include "bsp_usart.h"

#define LOG_I(fmt, ...) Printf("[INFO] " fmt, ##__VA_ARGS__)

#define BALANCE_CONTROL_TASK_PERIOD_MS 250 // 均衡控制任务周期，单位：ms秒

// 均衡参数，单位：mV
#define BMS_BALANCE_START_VOLTAGE_DIFF 70  // 均衡开始电压差阈值
#define BMS_BALANCE_STOP_VOLTAGE_DIFF 50   // 均衡停止电压差阈值
#define BMS_BALANCE_START_MIN_VOLTAGE 3400 // 均衡开始最小电压阈值

BMS_BalanceInfo bms_balance_info = {0, BMS_BALANCE_STATUS_IDLE};

static void BMS_BalanceControl(void);
static void BMS_StartBalance(void);
static void BMS_StopBalance(void);

// 电芯均衡控制
void BalanceControlTask(void *argument)
{
    for (;;)
    {
        BMS_BalanceControl();
        osDelay(BALANCE_CONTROL_TASK_PERIOD_MS);
    }
}

static void BMS_BalanceControl(void)
{
    static uint8_t start_balance_count = 0;
    static uint8_t stop_balance_count = 0;

    // 开始均衡条件：
    // 1. 使能均衡功能
    // 2. 系统充电状态
    // 3. 均衡空闲状态
    // 4. 电芯最小电压大于均衡开始最小电压阈值
    // 5. 电芯最大电压差大于均衡开始电压差阈值
    // 6. 连续4次均衡条件满足
    if (bms_user_config.balance == BMS_ENABLE)
    {
        if (bms_sys_state == BMS_STATE_CHARGING &&
            bms_balance_info.balance_status == BMS_BALANCE_STATUS_IDLE &&
            bms_data_acq.min_voltage >= BMS_BALANCE_START_MIN_VOLTAGE &&
            bms_data_acq.max_voltage_diff >= BMS_BALANCE_START_VOLTAGE_DIFF)
        {
            start_balance_count++;
            if (start_balance_count >= 4)
            {
                BMS_StartBalance();
                start_balance_count = 0;
                bms_balance_info.balance_status = BMS_BALANCE_STATUS_RUNNING;
                bms_balance_info.balance_cell_index = bms_data_acq.max_voltage_index;
            }
        }
        else
        {
            start_balance_count = 0;
        }
    }

    // 停止均衡条件：
    // 1. 均衡处于运行状态
    // 2. 电芯最大电压差小于均衡停止电压差阈值
    if (bms_balance_info.balance_status == BMS_BALANCE_STATUS_RUNNING)
    {
        if ((bms_data_acq.cell_voltages[bms_balance_info.balance_cell_index] -
             bms_data_acq.min_voltage) < BMS_BALANCE_STOP_VOLTAGE_DIFF)
        {
            stop_balance_count++;
            if (stop_balance_count >= 4)
            {
                BMS_StopBalance();
                bms_balance_info.balance_status = BMS_BALANCE_STATUS_IDLE;
                stop_balance_count = 0;
            }
        }
        else
        {
            stop_balance_count = 0;
        }
    }
}

static void BMS_StartBalance(void)
{
    uint8_t start_balance_cell_id = cell_id_index_map[bms_data_acq.max_voltage_index];
    // 开始均衡
    BQ76940_StartBalancing(start_balance_cell_id);
    LOG_I("Start balance cell ID: %d, index: %d\r\n", start_balance_cell_id, bms_balance_info.balance_cell_index);
}

static void BMS_StopBalance(void)
{
    uint8_t stop_balance_cell_id = cell_id_index_map[bms_balance_info.balance_cell_index];
    // 停止均衡
    BQ76940_StopBalancing(stop_balance_cell_id);
    LOG_I("Stop balance cell ID: %d, index: %d\r\n", stop_balance_cell_id, bms_balance_info.balance_cell_index);
}
