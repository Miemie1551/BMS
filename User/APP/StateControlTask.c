#include "StateControlTask.h"
#include "cmsis_os.h"

void StateControlTask(void *argument)
{
    for (;;)
    {
        osDelay(2000);
    }
}
