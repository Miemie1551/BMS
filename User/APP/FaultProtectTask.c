#include "FaultProtectTask.h"
#include "cmsis_os.h"

void FaultProtectTask(void *argument)
{
    for (;;)
    {
        osDelay(2000);
    }
}


