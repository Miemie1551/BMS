#include "BalanceControlTask.h"
#include "cmsis_os.h"

void BalanceControlTask(void *argument)
{
    for (;;)
    {
        osDelay(2000);
    }
}
