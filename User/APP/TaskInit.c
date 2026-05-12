#include "TaskInit.h"
#include "cmsis_os.h"

#include "FaultProtectTask.h"
#include "DataAcqTask.h"
#include "StateControlTask.h"
#include "BatteryGaugeTask.h"
#include "CommTask.h"
#include "BalanceControlTask.h"

osThreadId_t faultProtectTaskHandle;
const osThreadAttr_t faultProtectTask_attributes = {
    .name = "faultProtectTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal6,
};

osThreadId_t dataAcqTaskHandle;
const osThreadAttr_t dataAcqTask_attributes = {
    .name = "dataAcqTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal5,
};

osThreadId_t stateControlTaskHandle;
const osThreadAttr_t stateControlTask_attributes = {
    .name = "stateControlTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal4,
};

osThreadId_t batteryGaugeTaskHandle;
const osThreadAttr_t batteryGaugeTask_attributes = {
    .name = "batteryGaugeTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal3,
};

osThreadId_t commTaskHandle;
const osThreadAttr_t commTask_attributes = {
    .name = "commTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal2,
};

osThreadId_t balanceControlTaskHandle;
const osThreadAttr_t balanceControlTask_attributes = {
    .name = "balanceControlTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal1,
};

void TaskInit(void)
{
    faultProtectTaskHandle = osThreadNew(FaultProtectTask, NULL, &faultProtectTask_attributes);
    dataAcqTaskHandle = osThreadNew(DataAcqTask, NULL, &dataAcqTask_attributes);
    stateControlTaskHandle = osThreadNew(StateControlTask, NULL, &stateControlTask_attributes);
    batteryGaugeTaskHandle = osThreadNew(BatteryGaugeTask, NULL, &batteryGaugeTask_attributes);
    commTaskHandle = osThreadNew(CommTask, NULL, &commTask_attributes);
    balanceControlTaskHandle = osThreadNew(BalanceControlTask, NULL, &balanceControlTask_attributes);
}
