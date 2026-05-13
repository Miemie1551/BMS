#include "StateControlTask.h"
#include "cmsis_os.h"

#include "bms_global.h"


void StateControlTask(void *argument)
{
    for (;;)
    {
        osDelay(100);

    }
}

void StandbyStateHandler(void)
{
    // 待机状态处理逻辑
}
