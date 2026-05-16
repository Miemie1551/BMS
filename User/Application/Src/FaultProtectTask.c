#include "FaultProtectTask.h"
#include "cmsis_os.h"


// 过流/短路/过压/欠压/过温保护

void FaultProtectTask(void *argument)
{
    for (;;)
    {
        osDelay(2000);
    }
}


