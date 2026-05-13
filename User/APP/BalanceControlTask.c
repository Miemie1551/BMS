#include "BalanceControlTask.h"
#include "cmsis_os.h"

// 电芯均衡控制
void BalanceControlTask(void *argument)
{
    for (;;)
    {
        osDelay(2000);
    }
}
