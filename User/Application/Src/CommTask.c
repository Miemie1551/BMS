#include "CommTask.h"
#include "cmsis_os.h"


#include "bsp_usart.h"

// CAN/RS485通信
void CommTask(void *argument)
{
    for (;;)
    {
        osDelay(2000);
    }
}
