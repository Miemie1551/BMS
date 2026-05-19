#ifndef __DEV_BQ76940_H
#define __DEV_BQ76940_H

#include "stm32f1xx_hal.h"

// 电池单体数量
#define CELL_TOTAL 9

/* SYS_STAT位枚举变量定义 */
typedef enum
{
    STAT_ALL = 0xBF,                    // 所有位
    STAT_CC_READY_BIT = 0x01 << 7,      // 新的库仑计数器读数可用
    STAT_DEVICE_XREADY_BIT = 0x01 << 5, // 内部芯片故障
    STAT_OVRD_ALERT_BIT = 0x01 << 4,    // ALERT引脚触发（被拉至高电平）
    STAT_UV_BIT = 0x01 << 3,            // 欠压故障
    STAT_OV_BIT = 0x01 << 2,            // 过压故障
    STAT_SCD_BIT = 0x01 << 1,           // 放电短路故障
    STAT_OCD_BIT = 0x01 << 0            // 放电过流故障
} SYS_STAT_Bits;

extern const uint8_t cell_id_index_map[CELL_TOTAL];

uint8_t BQ76940_Init(void);

uint8_t BQ76940_ReadCellVoltages(uint16_t _voltages[]);
uint8_t BQ76940_ReadBatteryVoltage(uint16_t *_voltage);
uint8_t BQ76940_ReadCurrent(int16_t *_current);
uint8_t BQ76940_ReadTemperature(int16_t *_temperature);

uint8_t BQ76940_GetSystemStatus(uint8_t *status_bits);
uint8_t BQ76940_ClearFaults(uint8_t bit_mask);

uint8_t BQ76940_GetSystemControl2(uint8_t *ctrl2_bits);
uint8_t BQ76940_EnableDischarging(void);
uint8_t BQ76940_DisableDischarging(void);
uint8_t BQ76940_EnableCharging(void);
uint8_t BQ76940_DisableCharging(void);

void BQ76940_StartBalancing(uint8_t _cell_id);
void BQ76940_StopBalancing(uint8_t _cell_id);

#endif /* __DEV_BQ76940_H */
