#include "BatteryGaugeTask.h"
#include "cmsis_os.h"

// SOC/SOH计算、容量学习
void BatteryGaugeTask(void *argument)
{
    for (;;)
    {
        osDelay(2000);
    }
}
