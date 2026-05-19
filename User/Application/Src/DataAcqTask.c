#include "DataAcqTask.h"
#include "cmsis_os2.h"

#include "FaultProtectTask.h"
#include "StateControlTask.h"
#include "BatteryGaugeTask.h"
#include "BalanceControlTask.h"

#include "bsp_usart.h"
#include "dev_bq76940.h"

#define LOG_I(fmt, ...) Printf("[INFO] " fmt, ##__VA_ARGS__)
#define LOG_D(fmt, ...) Printf("[DEBUG] " fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...) Printf("[ERROR] [%s:%d]" fmt, __FILE__, __LINE__, ##__VA_ARGS__)

#define DATA_ACQ_TASK_PERIOD_MS 1000

BMS_DataAcq_t bms_data_acq;

static void CellParameter_Calculate(void);
static void BMS_PrintInfo(void);
static void BMS_StateOutput(void);
static void BMS_AlertOutput(void);

// 电芯电压/电流/温度采样
void DataAcqTask(void *argument)
{
    for (;;)
    {
        if (BQ76940_ReadCellVoltages(bms_data_acq.cell_voltages) != 1)
        {
            LOG_E("Failed to read cell voltages\r\n");
        }
        if (BQ76940_ReadBatteryVoltage(&bms_data_acq.battery_voltage) != 1)
        {
            LOG_E("Failed to read battery voltage\r\n");
        }
        if (BQ76940_ReadCurrent(&bms_data_acq.current) != 1)
        {
            LOG_E("Failed to read current\r\n");
        }
        if (BQ76940_ReadTemperature(&bms_data_acq.temperature) != 1)
        {
            LOG_E("Failed to read temperature\r\n");
        }

        CellParameter_Calculate();
        BMS_PrintInfo();

        osDelay(1000);
    }
}

static void CellParameter_Calculate(void)
{
    bms_data_acq.max_voltage = 0;
    bms_data_acq.min_voltage = 0xFFFF;
    uint16_t voltage = 0;

    for (int i = 0; i < BMS_NUM_CELLS; i++)
    {
        voltage = bms_data_acq.cell_voltages[i];

        if (voltage > bms_data_acq.max_voltage)
        {
            bms_data_acq.max_voltage = voltage;
            bms_data_acq.max_voltage_index = i;
        }
        if (voltage < bms_data_acq.min_voltage)
        {
            bms_data_acq.min_voltage = voltage;
        }
    }

    bms_data_acq.max_voltage_diff = bms_data_acq.max_voltage - bms_data_acq.min_voltage;
    bms_data_acq.avg_voltage = bms_data_acq.battery_voltage / BMS_NUM_CELLS;
}

static void BMS_PrintInfo(void)
{
    float soc_f = bms_battery_gauge_data.soc * 0.1f;       // 转换为%
    float current_a = bms_data_acq.current * 0.001f;       // 转换为A
    float temperature_c = bms_data_acq.temperature * 0.1f; // 转换为℃

    LOG_I("/******************* BMS Info *******************/\r\n");
    BMS_StateOutput();
    LOG_I("Battery SOC: %.1f%%\r\n", soc_f);

    LOG_I("Cell Max Voltage: %d.%03dV\r\n", bms_data_acq.max_voltage / 1000, bms_data_acq.max_voltage % 1000);
    LOG_I("Cell Min Voltage: %d.%03dV\r\n", bms_data_acq.min_voltage / 1000, bms_data_acq.min_voltage % 1000);
    LOG_I("Cell Max Voltage Difference: %01d.%03dV\r\n", bms_data_acq.max_voltage_diff / 1000, bms_data_acq.max_voltage_diff % 1000);
    LOG_I("Cell Avg Voltage: %d.%03dV\r\n\r\n", bms_data_acq.avg_voltage / 1000, bms_data_acq.avg_voltage % 1000);

    LOG_I("Battery Voltage: %d.%03dV\r\n", bms_data_acq.battery_voltage / 1000, bms_data_acq.battery_voltage % 1000);
    LOG_I("Battery Current: %.3fA\r\n", current_a);
    LOG_I("Battery Temperature: %.1f\r\n\r\n", temperature_c);

    LOG_I("Cell1 Voltage: %d.%03dV\r\n", bms_data_acq.cell_voltages[0] / 1000, bms_data_acq.cell_voltages[0] % 1000);
    LOG_I("Cell2 Voltage: %d.%03dV\r\n", bms_data_acq.cell_voltages[1] / 1000, bms_data_acq.cell_voltages[1] % 1000);
    LOG_I("Cell3 Voltage: %d.%03dV\r\n", bms_data_acq.cell_voltages[2] / 1000, bms_data_acq.cell_voltages[2] % 1000);
    LOG_I("Cell4 Voltage: %d.%03dV\r\n", bms_data_acq.cell_voltages[3] / 1000, bms_data_acq.cell_voltages[3] % 1000);
    LOG_I("Cell5 Voltage: %d.%03dV\r\n", bms_data_acq.cell_voltages[4] / 1000, bms_data_acq.cell_voltages[4] % 1000);
    LOG_I("Cell6 Voltage: %d.%03dV\r\n", bms_data_acq.cell_voltages[5] / 1000, bms_data_acq.cell_voltages[5] % 1000);
    LOG_I("Cell7 Voltage: %d.%03dV\r\n", bms_data_acq.cell_voltages[6] / 1000, bms_data_acq.cell_voltages[6] % 1000);
    LOG_I("Cell8 Voltage: %d.%03dV\r\n", bms_data_acq.cell_voltages[7] / 1000, bms_data_acq.cell_voltages[7] % 1000);
    LOG_I("Cell9 Voltage: %d.%03dV\r\n\r\n", bms_data_acq.cell_voltages[8] / 1000, bms_data_acq.cell_voltages[8] % 1000);

    LOG_I("CHG: %d; DSG: %d\r\n", bms_fet_state.state_CHG, bms_fet_state.state_DSG);
    if (bms_balance_info.balance_status == BMS_BALANCE_STATUS_RUNNING)
    {
        LOG_I("Balance Cell Index: %d\r\n", bms_balance_info.balance_cell_index);
    }
    LOG_I("/***********************************************/\r\n\r\n");
}

static void BMS_StateOutput(void)
{
    switch (bms_sys_state)
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
        BMS_AlertOutput();
        break;
    }
}

static void BMS_AlertOutput(void)
{
    switch (bms_protect_alert)
    {
    case FlAG_ALERT_OV:
        LOG_I("BMS Alert: Charging Overvoltage\r\n");
        break;
    case FlAG_ALERT_UV:
        LOG_I("BMS Alert: Discharging Undervoltage\r\n");
        break;
    case FlAG_ALERT_OCC:
        LOG_I("BMS Alert: Charging Overcurrent\r\n");
        break;
    case FlAG_ALERT_OCD:
        LOG_I("BMS Alert: Discharging Overcurrent\r\n");
        break;
    case FlAG_ALERT_SCD:
        LOG_I("BMS Alert: Discharging Short Circuit\r\n");
        break;
    case FlAG_ALERT_OTC:
        LOG_I("BMS Alert: Charging Overtemperature\r\n");
        break;
    case FlAG_ALERT_OTD:
        LOG_I("BMS Alert: Discharging Overtemperature\r\n");
        break;
    case FlAG_ALERT_LTC:
        LOG_I("BMS Alert: Charging Low Temperature\r\n");
        break;
    case FlAG_ALERT_LTD:
        LOG_I("BMS Alert: Discharging Low Temperature\r\n");
        break;
    case FlAG_ALERT_AFE_COMM:
        LOG_I("BMS Alert: AFE Communication Fault\r\n");
        break;
    }
}
