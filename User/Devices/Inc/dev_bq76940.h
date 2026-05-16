#ifndef __DEV_BQ76940_H
#define __DEV_BQ76940_H

#include "main.h"

/* SYS_STAT位枚举变量定义 */
typedef enum
{
    STAT_ALL = 0xBF,
    STAT_CC_READY_BIT = 0x01 << 7,
    STAT_DEVICE_XREADY_BIT = 0x01 << 5,
    STAT_OVRD_ALERT_BIT = 0x01 << 4,
    STAT_UV_BIT = 0x01 << 3,
    STAT_OV_BIT = 0x01 << 2,
    STAT_SCD_BIT = 0x01 << 1,
    STAT_OCD_BIT = 0x01 << 0
} SYS_STAT_Bits;

uint8_t BQ76940_Init(void);

uint8_t BQ76940_ReadCellVoltages(uint16_t _voltages[], uint16_t *_total_voltage);
uint8_t BQ76940_ReadCurrent(int16_t *_current);
uint8_t BQ76940_ReadTemperature(int16_t *_temperature);

uint8_t BQ76940_GetSystemStatus(uint8_t *status_bits);
uint8_t BQ76940_ClearFaults(SYS_STAT_Bits bit_mask);

uint8_t BQ76940_EnableDischarging(void);
uint8_t BQ76940_DisableDischarging(void);
uint8_t BQ76940_EnableCharging(void);
uint8_t BQ76940_DisableCharging(void);

void BQ76940_FindCellMaxVoltageID(uint16_t _voltages[], uint8_t *_max_voltage_id);
void BQ76940_StartBalancing(uint8_t _cell_id);
void BQ76940_StopBalancing(void);

#endif /* __DEV_BQ76940_H */
