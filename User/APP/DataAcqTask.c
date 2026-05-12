#include "DataAcqTask.h"
#include "cmsis_os2.h"

#include "bsp_bq76940.h"
#include "bms_global.h"
#include "usart.h"

#define LOG_I(fmt, ...) printf("[INFO] " fmt "\r\n", ##__VA_ARGS__)
#define LOG_E(fmt, ...) printf("[ERROR] " fmt "\r\n", ##__VA_ARGS__)

uint16_t cell_voltages[9] = {0};
uint16_t total_voltage = 0;
int16_t current = 0;
int16_t temperature = 0;

void DataAcqTask(void *argument)
{

    BQ76940_Init();

    for (;;)
    {
        osDelay(1000);
        if (BQ76940_ReadCellVoltages(cell_voltages, &total_voltage) == 1)
        {
            LOG_I("Cell Voltages: %d, %d, %d, %d, %d, %d, %d, %d, %d mV",
                  cell_voltages[0], cell_voltages[1], cell_voltages[2],
                  cell_voltages[3], cell_voltages[4], cell_voltages[5],
                  cell_voltages[6], cell_voltages[7], cell_voltages[8]);
            LOG_I("Total Voltage: %d mV", total_voltage);
        }
        else
        {
            LOG_E("Failed to read cell voltages");
        }
        if (BQ76940_ReadCurrent(&current) == 1)
        {
            LOG_I("Current: %d mA", current);
        }
        else
        {
            LOG_E("Failed to read current");
        }

        if (BQ76940_ReadTemperature(&temperature) == 1)
        {
            LOG_I("Temperature: %d.%d", temperature / 10, temperature % 10);
        }
        else
        {
            LOG_E("Failed to read temperature");
        }

    }
}
