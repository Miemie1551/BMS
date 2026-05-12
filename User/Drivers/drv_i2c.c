#include "drv_i2c.h"

static void I2C_Delay(void)
{
    volatile uint16_t delay = 50;
    while (delay--)
        __NOP();
}

static void I2C_WriteSCL(I2C_Handle_t *hi2c, uint8_t state)
{
    HAL_GPIO_WritePin(hi2c->I2C_Port, hi2c->SCL_Pin, (GPIO_PinState)state);
    I2C_Delay();
}

static void I2C_WriteSDA(I2C_Handle_t *hi2c, uint8_t state)
{
    HAL_GPIO_WritePin(hi2c->I2C_Port, hi2c->SDA_Pin, (GPIO_PinState)state);
    I2C_Delay();
}

static uint8_t I2C_ReadSDA(I2C_Handle_t *hi2c)
{
    uint8_t bit_value = HAL_GPIO_ReadPin(hi2c->I2C_Port, hi2c->SDA_Pin);
    I2C_Delay();
    return bit_value;
}

static void I2C_Start(I2C_Handle_t *hi2c)
{
    I2C_WriteSDA(hi2c, 1); // 兼容重复发送起始位
    I2C_WriteSCL(hi2c, 1);
    I2C_WriteSDA(hi2c, 0); // 在SCL为高电平时拉低SDA表示起始条件
    I2C_WriteSCL(hi2c, 0);
}
static void I2C_Stop(I2C_Handle_t *hi2c)
{
    I2C_WriteSDA(hi2c, 0);
    I2C_WriteSCL(hi2c, 1);
    I2C_WriteSDA(hi2c, 1); // 在SCL为高电平时拉高SDA表示停止条件
}

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

static uint8_t I2C_ReceiveAck(I2C_Handle_t *hi2c)
{
    I2C_WriteSDA(hi2c, 1);
    I2C_WriteSCL(hi2c, 1);
    uint8_t ack = I2C_ReadSDA(hi2c);
    I2C_WriteSCL(hi2c, 0);
    return ack;
}

static void I2C_SendAck(I2C_Handle_t *hi2c, uint8_t ack)
{
    I2C_WriteSDA(hi2c, ack);
    I2C_WriteSCL(hi2c, 1);
    I2C_WriteSCL(hi2c, 0);
}

/**
 * @brief Writes data to a specified register over I2C
 * @param hi2c I2C handle
 * @param reg_addr Register address
 * @param data Data to write
 * @param data_len Length of data to write
 * @return 1: success, 0: failure
 */
uint8_t I2C_Write(I2C_Handle_t *hi2c, uint8_t reg_addr, uint8_t *data, uint16_t data_len)
{
    if (data_len == 0 || data == NULL)
        return 0;

    I2C_Start(hi2c);

    I2C_SendByte(hi2c, hi2c->DeviceAddress << 1);
    if (I2C_ReceiveAck(hi2c) != 0)
    {
        I2C_Stop(hi2c);
        return 0; // No ACK
    }

    I2C_SendByte(hi2c, reg_addr);
    if (I2C_ReceiveAck(hi2c) != 0)
    {
        I2C_Stop(hi2c);
        return 0; // No ACK
    }

    for (uint16_t i = 0; i < data_len; i++)
    {
        I2C_SendByte(hi2c, data[i]);
        if (I2C_ReceiveAck(hi2c) != 0)
        {
            I2C_Stop(hi2c);
            return 0; // No ACK
        }
    }

    I2C_Stop(hi2c);
    return 1; // Success
}

/**
 * @brief Read specified register over I2C
 * @param hi2c I2C handle
 * @param reg_addr Register address
 * @param data Read data
 * @param data_len Read data length
 * @return 1: success, 0: failure
 */
uint8_t I2C_Read(I2C_Handle_t *hi2c, uint8_t reg_addr, uint8_t *data, uint16_t data_len)
{
    if (data_len == 0 || data == NULL)
        return 0;

    I2C_Start(hi2c);

    I2C_SendByte(hi2c, hi2c->DeviceAddress << 1);
    if (I2C_ReceiveAck(hi2c) != 0)
    {
        I2C_Stop(hi2c);
        return 0; // No ACK
    }

    I2C_SendByte(hi2c, reg_addr);
    if (I2C_ReceiveAck(hi2c) != 0)
    {
        I2C_Stop(hi2c);
        return 0; // No ACK
    }

    I2C_Start(hi2c);

    I2C_SendByte(hi2c, (hi2c->DeviceAddress << 1) | 0x01);
    if (I2C_ReceiveAck(hi2c) != 0)
    {
        I2C_Stop(hi2c);
        return 0; // No ACK
    }

    for (uint16_t i = 0; i < data_len; i++)
    {
        data[i] = I2C_ReceiveByte(hi2c);
        I2C_SendAck(hi2c, (data_len - 1 - i) ? 0 : 1);
    }

    I2C_Stop(hi2c);
    return 1; // Success
}
