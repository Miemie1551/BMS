#include "bsp_usart.h"
#include <stdarg.h>
#include <string.h>

#include "usart.h"
#include "cmsis_os.h"

#define PRINT_USART_HANDLE huart1
#define PRINT_USART_TIMEOUT 100

#define SuspendAll() osKernelLock()
#define ResumeAll() osKernelUnlock()

void Printf(char *format, ...)
{
    char buf[128];

    SuspendAll(); // 挂起调度器

    va_list ap;
    va_start(ap, format);

    if (vsprintf(buf, format, ap) > 0)
    {
        HAL_UART_Transmit(&PRINT_USART_HANDLE, (uint8_t *)buf, strlen(buf), PRINT_USART_TIMEOUT);
    }

    va_end(ap);

    ResumeAll(); // 恢复调度器
}
