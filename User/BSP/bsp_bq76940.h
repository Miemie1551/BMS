#ifndef __BSP_BQ76940_H
#define __BSP_BQ76940_H

#include "main.h"

uint8_t BQ76940_Init(void);
uint8_t BQ76940_ReadCellVoltages(uint16_t *voltages, uint16_t *total_voltage);
uint8_t BQ76940_ReadCurrent(int16_t *current);
uint8_t BQ76940_ReadTemperature(int16_t *temperature);

#endif /* __BSP_BQ76940_H */
