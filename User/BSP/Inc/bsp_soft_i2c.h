#ifndef __BSP_SOFT_I2C_H
#define __BSP_SOFT_I2C_H

#include "stm32f1xx_hal.h"

typedef struct
{
    GPIO_TypeDef *I2C_Port; // GPIO端口
    uint16_t SCL_Pin;       // SCL引脚
    uint16_t SDA_Pin;       // SDA引脚
    uint8_t DeviceAddress;  // 设备地址，不包含读写位
} I2C_Handle_t;

uint8_t I2C_Write(I2C_Handle_t *hi2c, uint8_t reg_addr, uint8_t *data, uint16_t data_len);
uint8_t I2C_Read(I2C_Handle_t *hi2c, uint8_t reg_addr, uint8_t *data, uint16_t data_len);

#endif /* __BSP_SOFT_I2C_H */
