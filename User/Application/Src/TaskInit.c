#include "TaskInit.h"
#include "cmsis_os.h"

#include "FaultProtectTask.h"
#include "DataAcqTask.h"
#include "StateControlTask.h"
#include "BatteryGaugeTask.h"
#include "CommTask.h"
#include "BalanceControlTask.h"

#include "dev_bq76940.h"
#include "bsp_usart.h"

#define LOG_E(fmt, ...) Printf("[ERROR] " fmt, ##__VA_ARGS__)

// 故障保护任务
osThreadId_t faultProtectTaskHandle;
const osThreadAttr_t faultProtectTask_attributes = {
    .name = "faultProtectTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal6,
};

// 数据采集任务
osThreadId_t dataAcqTaskHandle;
const osThreadAttr_t dataAcqTask_attributes = {
    .name = "dataAcqTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal5,
};

// 状态控制任务
osThreadId_t stateControlTaskHandle;
const osThreadAttr_t stateControlTask_attributes = {
    .name = "stateControlTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal4,
};

// 电池状态测量任务
osThreadId_t batteryGaugeTaskHandle;
const osThreadAttr_t batteryGaugeTask_attributes = {
    .name = "batteryGaugeTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal3,
};

// 通信任务
osThreadId_t commTaskHandle;
const osThreadAttr_t commTask_attributes = {
    .name = "commTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal2,
};

// 均衡控制任务
osThreadId_t balanceControlTaskHandle;
const osThreadAttr_t balanceControlTask_attributes = {
    .name = "balanceControlTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal1,
};

void TaskInit(void)
{
    BQ76940_Init(); // 初始化BQ76940

    osKernelLock(); // 挂起所有任务，只允许当前任务切换

    // 初始化任务
    faultProtectTaskHandle = osThreadNew(FaultProtectTask, NULL, &faultProtectTask_attributes);
    if (faultProtectTaskHandle == NULL)
    {
        LOG_E("FaultProtectTask create failed\r\n");
    }

    dataAcqTaskHandle = osThreadNew(DataAcqTask, NULL, &dataAcqTask_attributes);
    if (dataAcqTaskHandle == NULL)
    {
        LOG_E("DataAcqTask create failed\r\n");
    }

    stateControlTaskHandle = osThreadNew(StateControlTask, NULL, &stateControlTask_attributes);
    if (stateControlTaskHandle == NULL)
    {
        LOG_E("StateControlTask create failed\r\n");
    }

    batteryGaugeTaskHandle = osThreadNew(BatteryGaugeTask, NULL, &batteryGaugeTask_attributes);
    if (batteryGaugeTaskHandle == NULL)
    {
        LOG_E("BatteryGaugeTask create failed\r\n");
    }

    commTaskHandle = osThreadNew(CommTask, NULL, &commTask_attributes);
    if (commTaskHandle == NULL)
    {
        LOG_E("CommTask create failed\r\n");
    }

    balanceControlTaskHandle = osThreadNew(BalanceControlTask, NULL, &balanceControlTask_attributes);
    if (balanceControlTaskHandle == NULL)
    {
        LOG_E("BalanceControlTask create failed\r\n");
    }

    osKernelUnlock(); // 释放所有任务，允许任务切换
}
