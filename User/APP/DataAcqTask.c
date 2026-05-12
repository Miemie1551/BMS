#include "DataAcqTask.h"
#include "cmsis_os2.h"

#include "usart.h"
#include "bsp_bq76940.h"
#include "bms_global.h"

#define LOG_I(fmt, ...) printf("[INFO] " fmt "\r\n", ##__VA_ARGS__)
#define LOG_E(fmt, ...) printf("[ERROR] " fmt "\r\n", ##__VA_ARGS__)

void DataAcqTask(void *argument)
{
    BQ76940_Init();
    for (;;)
    {
        osDelay(1000);
        if (BQ76940_ReadCellVoltages(g_bms_data.cell_voltages_array, &g_bms_data.battery_voltage) == 1)
        {
            LOG_I("Cell Voltages: %d, %d, %d, %d, %d, %d, %d, %d, %d mV",
                  g_bms_data.cell_voltages_array[0], g_bms_data.cell_voltages_array[1], g_bms_data.cell_voltages_array[2],
                  g_bms_data.cell_voltages_array[3], g_bms_data.cell_voltages_array[4], g_bms_data.cell_voltages_array[5],
                  g_bms_data.cell_voltages_array[6], g_bms_data.cell_voltages_array[7], g_bms_data.cell_voltages_array[8]);
            LOG_I("Total Voltage: %d mV", g_bms_data.battery_voltage);
        }
        else
        {
            LOG_E("Failed to read cell voltages");
        }
        if (BQ76940_ReadCurrent(&g_bms_data.battery_current) == 1)
        {
            LOG_I("Current: %d mA", g_bms_data.battery_current);
        }
        else
        {
            LOG_E("Failed to read current");
        }

        if (BQ76940_ReadTemperature(&g_bms_data.temperature) == 1)
        {
            LOG_I("Temperature: %d.%d", g_bms_data.temperature / 10, g_bms_data.temperature % 10);
        }
        else
        {
            LOG_E("Failed to read temperature");
        }

    }
}
