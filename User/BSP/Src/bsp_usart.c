#include "bsp_usart.h"
#include <stdarg.h>
#include <string.h>

#include "usart.h"
#include "cmsis_os.h"

#define PRINT_USART_HANDLE huart1
#define PRINT_USART_TIMEOUT 100

#define ENTER_CRITICAL() portENTER_CRITICAL()
#define EXIT_CRITICAL() portEXIT_CRITICAL()

void Printf(char *format, ...)
{
    char buf[64];

    ENTER_CRITICAL();

    va_list ap;
    va_start(ap, format);

    if (vsprintf(buf, format, ap) > 0)
    {
        HAL_UART_Transmit(&PRINT_USART_HANDLE, (uint8_t *)buf, strlen(buf), PRINT_USART_TIMEOUT);
    }

    va_end(ap);

    EXIT_CRITICAL();
}
