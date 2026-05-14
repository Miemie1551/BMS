#include "DataAcqTask.h"
#include "cmsis_os2.h"

#include "bms_global.h"

#include "usart.h"
#include "bsp_bq76940.h"

#define LOG_I(fmt, ...) printf("[INFO] " fmt, ##__VA_ARGS__)
#define LOG_D(fmt, ...) printf("[DEBUG] " fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...) printf("[ERROR] [%s:%d]" fmt, __FILE__, __LINE__, ##__VA_ARGS__)

static BMS_Info_t local_bms_info;

static void SOC_Calculation(void);
static void CellVoltageAnalysis(void);

static void BMS_PrintInfo(void);
static void BMS_StateOutput(void);
static void BMS_FaultOutput(void);

// 电芯电压/电流/温度采样
void DataAcqTask(void *argument)
{
    BMS_Info_Init();
    BQ76940_Init();

    for (;;)
    {
        // 读取电池信息
        BMS_Info_Read(&local_bms_info);

        if (BQ76940_ReadCellVoltages(local_bms_info.cell_data.voltages, &local_bms_info.pack_data.voltage) != 1)
        {
            LOG_E("Failed to read cell voltages\r\n");
        }
        if (BQ76940_ReadCurrent(&local_bms_info.pack_data.current) != 1)
        {
            LOG_E("Failed to read current\r\n");
        }

        if (BQ76940_ReadTemperature(&local_bms_info.pack_data.temperature) != 1)
        {
            LOG_E("Failed to read temperature\r\n");
        }
        SOC_Calculation();
        CellVoltageAnalysis();

        BMS_Info_Write(&local_bms_info);

        BMS_PrintInfo();
        osDelay(1000);
    }
}

static void SOC_Calculation(void)
{
    // 简单的SOC计算示例，实际应用中应使用更复杂的算法
    uint16_t total_voltage = local_bms_info.pack_data.voltage;
    if (total_voltage >= BMS_PACK_MAX_VOLTAGE)
        local_bms_info.soc = 100;
    else if (total_voltage >= BMS_PACK_MIN_VOLTAGE)
        local_bms_info.soc = (total_voltage - BMS_PACK_MIN_VOLTAGE) * 100 / (BMS_PACK_MAX_VOLTAGE - BMS_PACK_MIN_VOLTAGE);
    else
        local_bms_info.soc = 0;
}

static void CellVoltageAnalysis(void)
{
    local_bms_info.cell_data.max_voltage = 0;
    local_bms_info.cell_data.min_voltage = 0xFFFF;
    uint16_t voltage = 0;

    for (int i = 0; i < BMS_NUM_CELLS; i++)
    {
        voltage = local_bms_info.cell_data.voltages[i];

        if (voltage > local_bms_info.cell_data.max_voltage)
        {
            local_bms_info.cell_data.max_voltage = voltage;
        }
        if (voltage < local_bms_info.cell_data.min_voltage)
        {
            local_bms_info.cell_data.min_voltage = voltage;
        }
    }

    local_bms_info.cell_data.voltage_dif = local_bms_info.cell_data.max_voltage - local_bms_info.cell_data.min_voltage;
    local_bms_info.cell_data.avg_voltage = local_bms_info.pack_data.voltage / BMS_NUM_CELLS;
}

static void BMS_PrintInfo(void)
{
    float current_a = local_bms_info.pack_data.current / 1000.0f;       // 转换为A
    float temperature_c = local_bms_info.pack_data.temperature / 10.0f; // 转换为℃

    LOG_I("/******************* BMS Info *******************/\r\n");
    BMS_StateOutput();
    LOG_I("Battery SOC: %d%%\r\n\r\n", local_bms_info.soc);

    LOG_I("Cell Max Voltage: %d.%03dV\r\n", local_bms_info.cell_data.max_voltage / 1000, local_bms_info.cell_data.max_voltage % 1000);
    LOG_I("Cell Min Voltage: %d.%03dV\r\n", local_bms_info.cell_data.min_voltage / 1000, local_bms_info.cell_data.min_voltage % 1000);
    LOG_I("Cell Max Voltage Difference: %01d.%03dV\r\n", local_bms_info.cell_data.voltage_dif / 1000, local_bms_info.cell_data.voltage_dif % 1000);
    LOG_I("Cell Avg Voltage: %d.%03dV\r\n\r\n", local_bms_info.cell_data.avg_voltage / 1000, local_bms_info.cell_data.avg_voltage % 1000);

    LOG_I("Battery Voltage: %d.%03dV\r\n", local_bms_info.pack_data.voltage / 1000, local_bms_info.pack_data.voltage % 1000);
    LOG_I("Battery Current: %.3fA\r\n", current_a);
    LOG_I("Battery Temperature: %.1f\r\n\r\n", temperature_c);

    LOG_I("Cell1 Voltage: %d.%03dV\r\n", local_bms_info.cell_data.voltages[0] / 1000, local_bms_info.cell_data.voltages[0] % 1000);
    LOG_I("Cell2 Voltage: %d.%03dV\r\n", local_bms_info.cell_data.voltages[1] / 1000, local_bms_info.cell_data.voltages[1] % 1000);
    LOG_I("Cell3 Voltage: %d.%03dV\r\n", local_bms_info.cell_data.voltages[2] / 1000, local_bms_info.cell_data.voltages[2] % 1000);
    LOG_I("Cell4 Voltage: %d.%03dV\r\n", local_bms_info.cell_data.voltages[3] / 1000, local_bms_info.cell_data.voltages[3] % 1000);
    LOG_I("Cell5 Voltage: %d.%03dV\r\n", local_bms_info.cell_data.voltages[4] / 1000, local_bms_info.cell_data.voltages[4] % 1000);
    LOG_I("Cell6 Voltage: %d.%03dV\r\n", local_bms_info.cell_data.voltages[5] / 1000, local_bms_info.cell_data.voltages[5] % 1000);
    LOG_I("Cell7 Voltage: %d.%03dV\r\n", local_bms_info.cell_data.voltages[6] / 1000, local_bms_info.cell_data.voltages[6] % 1000);
    LOG_I("Cell8 Voltage: %d.%03dV\r\n", local_bms_info.cell_data.voltages[7] / 1000, local_bms_info.cell_data.voltages[7] % 1000);
    LOG_I("Cell9 Voltage: %d.%03dV\r\n", local_bms_info.cell_data.voltages[8] / 1000, local_bms_info.cell_data.voltages[8] % 1000);
    LOG_I("/***********************************************/\r\n\r\n");
}

static void BMS_StateOutput(void)
{
    switch (local_bms_info.state)
    {
    case BMS_STATE_STANDBY:
        LOG_I("BMS State: Standby\r\n");
        break;
    case BMS_STATE_CHARGING:
        LOG_I("BMS State: Charging\r\n");
        break;
    case BMS_STATE_DISCHARGING:
        LOG_I("BMS State: Discharging\r\n");
        break;
    case BMS_STATE_FAULT:
        LOG_I("BMS State: Fault\r\n");
        BMS_FaultOutput();
        break;
    }
}

static void BMS_FaultOutput(void)
{
    switch (local_bms_info.fault)
    {
    case FAULT_CELL_OV:
        LOG_I("BMS Fault: Cell Overvoltage\r\n");
    case FAULT_CELL_UV:
        LOG_I("BMS Fault: Cell Undervoltage\r\n");
    case FAULT_PACK_OV:
        LOG_I("BMS Fault: Pack Overvoltage\r\n");
    case FAULT_PACK_UV:
        LOG_I("BMS Fault: Pack Undervoltage\r\n");

    case FAULT_CHG_OC:
        LOG_I("BMS Fault: Charge Overcurrent\r\n");
    case FAULT_DSG_OC:
        LOG_I("BMS Fault: Discharge Overcurrent\r\n");
    case FAULT_SHORT_CIRCUIT:
        LOG_I("BMS Fault: Short Circuit\r\n");
    case FAULT_OVERTEMP:
        LOG_I("BMS Fault: Overtemperature\r\n");

    case FAULT_UNDERTEMP:
        LOG_I("BMS Fault: Undertemperature\r\n");
    case FAULT_AFE_COMM:
        LOG_I("BMS Fault: AFE Communication Fault\r\n");
    }
}
