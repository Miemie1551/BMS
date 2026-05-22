#include "bsp_usart.h"
#include <stdarg.h>
#include <stdio.h>

#include "usart.h"
#include "cmsis_os.h"

#define PRINT_USART_HANDLE huart1
#define PRINT_USART_TX_TIMEOUT 100
#define PRINT_BUF_SIZE 128

#define SuspendAll() osKernelLock()
#define ResumeAll() osKernelUnlock()

static char print_buf[PRINT_BUF_SIZE];

void Printf(const char *format, ...)
{
    // 挂起调度器
    SuspendAll();

    va_list ap;
    va_start(ap, format);

    // 获取格式化后的字符串长度，不包括 '\0' 字符
    int len = vsnprintf(print_buf, PRINT_BUF_SIZE, format, ap);

    // 安全检查，确保 len 不超过缓冲区大小
    if (len > 0 && len < PRINT_BUF_SIZE)
    {
        HAL_UART_Transmit(&PRINT_USART_HANDLE, (uint8_t *)print_buf, len, PRINT_USART_TX_TIMEOUT);
    }

    va_end(ap);

    ResumeAll(); // 恢复调度器
}

// snprintf()关键特性：
// ✅ 总是在字符串末尾添加 '\0'
// ✅ 如果字符串长度 >= size，会截断并确保末尾是 '\0'
// ✅ 如果字符串长度 >= size，返回值是格式化前的字符串长度（不包括 '\0'）
// ✅ 如果字符串长度 < size，返回值是格式化后的字符串长度（不包括 '\0'）
// ⚠️ 只覆盖到字符串末尾（包括 '\0'），后面的缓冲区内容不变

// 记忆口诀：
// 返回值 = 应该写入的字符数（不包括 '\0'）
// 实际写入 = min(返回值, size - 1) 个字符 + '\0'
// 截断判断 = 返回值 >= size
