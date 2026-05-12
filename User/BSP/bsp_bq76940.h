#ifndef __BSP_BQ76940_H
#define __BSP_BQ76940_H

#include "main.h"

/* SYS_STAT位枚举变量定义 */
typedef enum
{
    STAT_CC_READY_BIT = 1 << 7,
    STAT_DEVICE_XREADY_BIT = 1 << 5,
    STAT_OVRD_ALERT_BIT = 1 << 4,
    STAT_UV_BIT = 1 << 3,
    STAT_OV_BIT = 1 << 2,
    STAT_SCD_BIT = 1 << 1,
    STAT_OCD_BIT = 1 << 0
} SYS_STAT_Bits;

uint8_t BQ76940_Init(void);

uint8_t BQ76940_ReadCellVoltages(uint16_t _voltages[], uint16_t *_total_voltage);
uint8_t BQ76940_ReadCurrent(int16_t *_current);
uint8_t BQ76940_ReadTemperature(int16_t *_temperature);

uint8_t BQ76940_GetSystemStatus(uint8_t *status_bits);
uint8_t BQ76940_ClearFaults(SYS_STAT_Bits bit_mask);

uint8_t BQ76940_OpenDSG(void);
uint8_t BQ76940_CloseDSG(void);
uint8_t BQ76940_OpenCHG(void);
uint8_t BQ76940_CloseCHG(void);

#endif /* __BSP_BQ76940_H */
