#include "BalanceControlTask.h"
#include "cmsis_os.h"

#include "BMSInfo.h"

#include "dev_bq76940.h"

// 均衡参数，单位：mV
#define BMS_BALANCE_VOLTAGE 50 // 均衡开始电压差阈值

// 电芯均衡控制
void BalanceControlTask(void *argument)
{
    for (;;)
    {
        osDelay(2000);
    }
}
