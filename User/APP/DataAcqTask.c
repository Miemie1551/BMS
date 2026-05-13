#include "DataAcqTask.h"
#include "cmsis_os2.h"

#include "bms_global.h"

#include "usart.h"
#include "bsp_bq76940.h"

#define LOG_I(fmt, ...) printf("[INFO] " fmt, ##__VA_ARGS__)
#define LOG_D(fmt, ...) printf("[DEBUG] " fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...) printf("[ERROR] [%s:%d]" fmt, __FILE__, __LINE__, ##__VA_ARGS__)

static void SOC_Calculation(void);
static void Cell_Voltage_Analysis(void);
static void PrintData(void);

void DataAcqTask(void *argument)
{
    BQ76940_Init();
    for (;;)
    {
        osDelay(1000);
        if (BQ76940_ReadCellVoltages(g_bms_data.cell_voltages_array, &g_bms_data.battery_voltage) != 1)
        {
            LOG_E("Failed to read cell voltages\r\n");
        }
        if (BQ76940_ReadCurrent(&g_bms_data.battery_current) != 1)
        {
            LOG_E("Failed to read current\r\n");
        }

        if (BQ76940_ReadTemperature(&g_bms_data.temperature) != 1)
        {
            LOG_E("Failed to read temperature\r\n");
        }
        SOC_Calculation();
        Cell_Voltage_Analysis();
        PrintData();
    }
}

static void SOC_Calculation(void)
{
    // 简单的SOC计算示例，实际应用中应使用更复杂的算法
    uint16_t total_voltage = g_bms_data.battery_voltage;
    if (total_voltage >= BMS_BATTERY_VOLTAGE_MAX)
        g_bms_data.battery_soc = 100;
    else if (total_voltage >= BMS_BATTERY_VOLTAGE_MIN)
        g_bms_data.battery_soc = (total_voltage - BMS_BATTERY_VOLTAGE_MIN) * 100 / (BMS_BATTERY_VOLTAGE_MAX - BMS_BATTERY_VOLTAGE_MIN);
    else
        g_bms_data.battery_soc = 0;
}

static void Cell_Voltage_Analysis(void)
{
    g_bms_data.cell_max_voltage = 0;
    g_bms_data.cell_min_voltage = 0xFFFF;
    uint16_t voltage = 0;

    for (int i = 0; i < BMS_NUM_CELLS; i++)
    {
        voltage = g_bms_data.cell_voltages_array[i];

        if (voltage > g_bms_data.cell_max_voltage)
        {
            g_bms_data.cell_max_voltage = voltage;
        }
        if (voltage < g_bms_data.cell_min_voltage)
        {
            g_bms_data.cell_min_voltage = voltage;
        }
    }

    g_bms_data.cell_max_voltage_difference = g_bms_data.cell_max_voltage - g_bms_data.cell_min_voltage;
    g_bms_data.cell_avg_voltage = g_bms_data.battery_voltage / BMS_NUM_CELLS;
}

static void PrintData(void)
{
    float current_a = g_bms_data.battery_current / 1000.0f; // 转换为A
    float temperature_c = g_bms_data.temperature / 10.0f;   // 转换为℃

    LOG_I("/******************* BMS Data *******************/\r\n");
    LOG_I("Battery SOC: %d%%\r\n\r\n", g_bms_data.battery_soc);

    LOG_I("Cell Max Voltage: %d.%03dV\r\n", g_bms_data.cell_max_voltage / 1000, g_bms_data.cell_max_voltage % 1000);
    LOG_I("Cell Min Voltage: %d.%03dV\r\n", g_bms_data.cell_min_voltage / 1000, g_bms_data.cell_min_voltage % 1000);
    LOG_I("Cell Max Voltage Difference: %01d.%03dV\r\n", g_bms_data.cell_max_voltage_difference / 1000, g_bms_data.cell_max_voltage_difference % 1000);
    LOG_I("Cell Avg Voltage: %d.%03dV\r\n\r\n", g_bms_data.cell_avg_voltage / 1000, g_bms_data.cell_avg_voltage % 1000);

    LOG_I("Battery Voltage: %d.%03dV\r\n", g_bms_data.battery_voltage / 1000, g_bms_data.battery_voltage % 1000);
    LOG_I("Battery Current: %.3fA\r\n", current_a);
    LOG_I("Battery Temperature: %.1f\r\n\r\n", temperature_c);

    LOG_I("Cell1 Voltage: %d.%03dV\r\n", g_bms_data.cell_voltages_array[0] / 1000, g_bms_data.cell_voltages_array[0] % 1000);
    LOG_I("Cell2 Voltage: %d.%03dV\r\n", g_bms_data.cell_voltages_array[1] / 1000, g_bms_data.cell_voltages_array[1] % 1000);
    LOG_I("Cell3 Voltage: %d.%03dV\r\n", g_bms_data.cell_voltages_array[2] / 1000, g_bms_data.cell_voltages_array[2] % 1000);
    LOG_I("Cell4 Voltage: %d.%03dV\r\n", g_bms_data.cell_voltages_array[3] / 1000, g_bms_data.cell_voltages_array[3] % 1000);
    LOG_I("Cell5 Voltage: %d.%03dV\r\n", g_bms_data.cell_voltages_array[4] / 1000, g_bms_data.cell_voltages_array[4] % 1000);
    LOG_I("Cell6 Voltage: %d.%03dV\r\n", g_bms_data.cell_voltages_array[5] / 1000, g_bms_data.cell_voltages_array[5] % 1000);
    LOG_I("Cell7 Voltage: %d.%03dV\r\n", g_bms_data.cell_voltages_array[6] / 1000, g_bms_data.cell_voltages_array[6] % 1000);
    LOG_I("Cell8 Voltage: %d.%03dV\r\n", g_bms_data.cell_voltages_array[7] / 1000, g_bms_data.cell_voltages_array[7] % 1000);
    LOG_I("Cell9 Voltage: %d.%03dV\r\n", g_bms_data.cell_voltages_array[8] / 1000, g_bms_data.cell_voltages_array[8] % 1000);
    LOG_I("/***********************************************/\r\n\r\n");
}
