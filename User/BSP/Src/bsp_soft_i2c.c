#include "bsp_soft_i2c.h"
#include "cmsis_os.h"

// I2C延时时间，单位：微秒
#define I2C_DELAY_US 5

static void I2C_Delay(void);
static void I2C_EnterCritical(void);
static void I2C_ExitCritical(void);

static void I2C_WriteSCL(I2C_Handle_t *hi2c, uint8_t state);
static void I2C_WriteSDA(I2C_Handle_t *hi2c, uint8_t state);
static uint8_t I2C_ReadSDA(I2C_Handle_t *hi2c);

static void I2C_Start(I2C_Handle_t *hi2c);
static void I2C_Stop(I2C_Handle_t *hi2c);

static void I2C_SendByte(I2C_Handle_t *hi2c, uint8_t byte);
static uint8_t I2C_ReceiveByte(I2C_Handle_t *hi2c);

static void I2C_SendAck(I2C_Handle_t *hi2c, uint8_t ack);
static uint8_t I2C_ReceiveAck(I2C_Handle_t *hi2c);

/**
 * @brief 延时函数
 * @retval None
 */
static void I2C_Delay(void)
{
    uint16_t delay_us = I2C_DELAY_US;
    uint8_t i = 0;

    while (delay_us--)
    {
        i = 10;
        while (i--)
            __NOP();
    }
}

// 适用于72MHZ
// static void delay_us(uint32_t us)
// {
// 	uint16_t i = 0;

// 	while(us--)
// 	{
// 		i = 10; //自己定义
// 		while(i--);
// 	}
// }

/**
 * @brief 进入临界区
 * @retval None
 */
static void I2C_EnterCritical(void)
{
    portENTER_CRITICAL(); // 进入临界区
}

/**
 * @brief 退出临界区
 * @retval None
 */
static void I2C_ExitCritical(void)
{
    portEXIT_CRITICAL(); // 退出临界区
}

/**
 * @brief 写入SCL引脚状态
 * @param hi2c I2C handle
 * @param state 0: 低电平, 1: 高电平
 * @retval None
 */
static void I2C_WriteSCL(I2C_Handle_t *hi2c, uint8_t state)
{
    HAL_GPIO_WritePin(hi2c->I2C_Port, hi2c->SCL_Pin, (GPIO_PinState)state);
    I2C_Delay();
}

/**
 * @brief 写入SDA引脚状态
 * @param hi2c I2C handle
 * @param state 0: 低电平, 1: 高电平
 * @retval None
 */
static void I2C_WriteSDA(I2C_Handle_t *hi2c, uint8_t state)
{
    HAL_GPIO_WritePin(hi2c->I2C_Port, hi2c->SDA_Pin, (GPIO_PinState)state);
    I2C_Delay();
}

/**
 * @brief 读取SDA引脚状态
 * @param hi2c I2C handle
 * @retval 0: 低电平, 1: 高电平
 */
static uint8_t I2C_ReadSDA(I2C_Handle_t *hi2c)
{
    uint8_t bit_value = HAL_GPIO_ReadPin(hi2c->I2C_Port, hi2c->SDA_Pin);
    I2C_Delay();
    return bit_value;
}

/**
 * @brief 发送起始条件
 * @param hi2c I2C handle
 * @retval None
 */
static void I2C_Start(I2C_Handle_t *hi2c)
{
    I2C_WriteSDA(hi2c, 1); // 兼容重复发送起始位
    I2C_WriteSCL(hi2c, 1);
    I2C_WriteSDA(hi2c, 0); // 在SCL为高电平时拉低SDA表示起始条件
    I2C_WriteSCL(hi2c, 0);
}

/**
 * @brief 发送停止条件
 * @param hi2c I2C handle
 * @retval None
 */
static void I2C_Stop(I2C_Handle_t *hi2c)
{
    I2C_WriteSDA(hi2c, 0);
    I2C_WriteSCL(hi2c, 1);
    I2C_WriteSDA(hi2c, 1); // 在SCL为高电平时拉高SDA表示停止条件
}

/**
 * @brief 发送1个字节
 * @param hi2c I2C handle
 * @param byte Byte to send
 * @retval None
 */
static void I2C_SendByte(I2C_Handle_t *hi2c, uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        I2C_WriteSDA(hi2c, byte & 0x80); // 发送最高位
        byte <<= 1;                      // 左移准备发送下一位
        I2C_WriteSCL(hi2c, 1);
        I2C_WriteSCL(hi2c, 0);
    }
}

/**
 * @brief 接收1个字节
 * @param hi2c I2C handle
 * @retval 接收的字节值（高位在前）
 */
static uint8_t I2C_ReceiveByte(I2C_Handle_t *hi2c)
{
    uint8_t byte = 0;
    for (uint8_t i = 0; i < 8; i++)
    {
        I2C_WriteSCL(hi2c, 1);
        byte <<= 1;
        if (I2C_ReadSDA(hi2c))
            byte |= 0x01;
        I2C_WriteSCL(hi2c, 0);
    }
    return byte;
}

/**
 * @brief 发送ACK
 * @param hi2c I2C handle
 * @param ack 0: ACK, 1: NACK
 * @retval None
 */
static void I2C_SendAck(I2C_Handle_t *hi2c, uint8_t ack)
{
    I2C_WriteSDA(hi2c, ack);
    I2C_WriteSCL(hi2c, 1);
    I2C_WriteSCL(hi2c, 0);
}

/**
 * @brief 接收ACK
 * @param hi2c I2C handle
 * @retval 0: ACK, 1: NACK
 */
static uint8_t I2C_ReceiveAck(I2C_Handle_t *hi2c)
{
    I2C_WriteSDA(hi2c, 1);
    I2C_WriteSCL(hi2c, 1);
    uint8_t ack = I2C_ReadSDA(hi2c);
    I2C_WriteSCL(hi2c, 0);
    return ack;
}

/**
 * @brief I2C 写数据
 * @param hi2c I2C handle
 * @param reg_addr 寄存器地址
 * @param data 数据指针
 * @param data_len 数据长度
 * @retval 1: 成功, 0: 失败
 */
uint8_t I2C_Write(I2C_Handle_t *hi2c, uint8_t reg_addr, uint8_t *data, uint16_t data_len)
{
    if (hi2c == NULL || data_len == 0 || data == NULL)
        return 0;

    I2C_EnterCritical(); // 进入临界区
    I2C_Start(hi2c);

    I2C_SendByte(hi2c, hi2c->DeviceAddress << 1);
    if (I2C_ReceiveAck(hi2c) != 0)
    {
        goto error;
    }

    I2C_SendByte(hi2c, reg_addr);
    if (I2C_ReceiveAck(hi2c) != 0)
    {
        goto error;
    }

    for (uint16_t i = 0; i < data_len; i++)
    {
        I2C_SendByte(hi2c, data[i]);
        if (I2C_ReceiveAck(hi2c) != 0)
        {
            goto error;
        }
    }

    I2C_Stop(hi2c);
    I2C_ExitCritical(); // 退出临界区
    return 1;           // Success

error:
    // 错误处理
    I2C_Stop(hi2c);
    I2C_ExitCritical(); // 退出临界区
    return 0;           // No ACK
}

/**
 * @brief I2C 读数据
 * @param hi2c I2C 句柄
 * @param reg_addr 寄存器地址
 * @param data 数据指针
 * @param data_len 数据长度
 * @return 1: success, 0: failure
 */
uint8_t I2C_Read(I2C_Handle_t *hi2c, uint8_t reg_addr, uint8_t *data, uint16_t data_len)
{
    if (hi2c == NULL || data_len == 0 || data == NULL)
        return 0;

    I2C_EnterCritical(); // 进入临界区
    I2C_Start(hi2c);

    I2C_SendByte(hi2c, hi2c->DeviceAddress << 1);
    if (I2C_ReceiveAck(hi2c) != 0)
    {
        goto error;
    }

    I2C_SendByte(hi2c, reg_addr);
    if (I2C_ReceiveAck(hi2c) != 0)
    {
        goto error;
    }

    I2C_Start(hi2c);

    I2C_SendByte(hi2c, (hi2c->DeviceAddress << 1) | 0x01);
    if (I2C_ReceiveAck(hi2c) != 0)
    {
        goto error;
    }

    for (uint16_t i = 0; i < data_len; i++)
    {
        data[i] = I2C_ReceiveByte(hi2c);
        I2C_SendAck(hi2c, (data_len - 1 - i) ? 0 : 1);
    }

    I2C_Stop(hi2c);
    I2C_ExitCritical(); // 退出临界区
    return 1;           // Success

error:
    I2C_ExitCritical(); // 退出临界区
    return 0;           // No ACK
}
