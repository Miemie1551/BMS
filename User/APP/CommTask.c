#include "CommTask.h"
#include "cmsis_os.h"

void CommTask(void *argument)
{
    for (;;)
    {
        osDelay(2000);
    }
}
