#include "BatteryGaugeTask.h"
#include "cmsis_os.h"

void BatteryGaugeTask(void *argument)
{
    for (;;)
    {
        osDelay(2000);
    }
}
