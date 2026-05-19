#include "FaultProtectTask.h"
#include "cmsis_os.h"

#include "BMSInfo.h"
#include "DataAcqTask.h"
#include "StateControlTask.h"

#include "dev_bq76940.h"
#include "bsp_usart.h"

#define LOG_I(fmt, ...) Printf("[INFO] " fmt, ##__VA_ARGS__)
#define LOG_W(fmt, ...) Printf("[WARNING] " fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...) Printf("[ERROR] " fmt, ##__VA_ARGS__)

#define FAULT_PROTECT_TASK_PERIOD_MS 200

BMS_ProtectAlert_t bms_protect_alert;
static BMS_ProtectParams_t bms_protect_params =
    {
        .OV_Relieve = BMS_OV_RELIEVE,
        .UV_Relieve = BMS_UV_RELIEVE,

        .OCC_Protect = BMS_OCC_PROTECT,
        .OCC_RelieveTime = BMS_OCC_RELIEVE_TIME,
        .OCC_Delay = BMS_OCC_DELAY,

        .OCD_RelieveTime = BMS_OCD_RELIEVE_TIME,
        .SCD_RelieveTime = BMS_SCD_RELIEVE_TIME,

        .LTC_Protect = BMS_LTC_PROTECT,
        .LTC_Relieve = BMS_LTC_RELIEVE,
        .LTD_Protect = BMS_LTD_PROTECT,
        .LTD_Relieve = BMS_LTD_RELIEVE,

        .OTC_Protect = BMS_OTC_PROTECT,
        .OTC_Relieve = BMS_OTC_RELIEVE,
        .OTD_Protect = BMS_OTD_PROTECT,
        .OTD_Relieve = BMS_OTD_RELIEVE,
};

static BMS_ProtectState_t bms_protect_state = PROTECT_STATE_MONITOR;

static osTimerId_t pTimerProtect = NULL;
const static osTimerAttr_t protectTimer_attributes = {
    .name = "ProtectTimer",
};

static void BMS_ProtectSWMonitor(void);
static void BMS_ProtectHWMonitor(void);
static void BMS_ProtectRelieveWait(void);
static void BMS_ProtectRelieve(void);
static void BMS_ProtectTimerEntry(void *argument);
static void BMS_ProtectStartTimer(uint32_t sec);

// 充电过流、过温、低温保护；放电过温、低温保护
void FaultProtectTask(void *argument)
{
    pTimerProtect = osTimerNew(BMS_ProtectTimerEntry, osTimerOnce, NULL, &protectTimer_attributes);

    if (pTimerProtect == NULL)
    {
        LOG_E("Create Timer Fail\r\n");
    }

    for (;;)
    {
        switch (bms_protect_state)
        {
        case PROTECT_STATE_MONITOR:
            BMS_ProtectSWMonitor();
            BMS_ProtectHWMonitor();
            break;
        case PROTECT_STATE_RELIEVE_WAIT:
            BMS_ProtectRelieveWait();
            break;
        case PROTECT_STATE_RELIEVE:
            BMS_ProtectRelieve();
            break;
        }

        osDelay(FAULT_PROTECT_TASK_PERIOD_MS);
    }
}

static void BMS_ProtectSWMonitor(void)
{
    switch (bms_state)
    {
    case BMS_STATE_CHARGING:
        // 充电过流保护
        if (bms_data_acq.current > bms_protect_params.OCC_Protect)
        {
            BMS_ControlCharge(0);
            bms_protect_alert = FlAG_ALERT_OCC;
            bms_protect_state = PROTECT_STATE_RELIEVE_WAIT;
            BMS_ProtectStartTimer(bms_protect_params.OCC_RelieveTime);
            LOG_W("OCC Fault\r\n");
        }
        // 充电过温保护
        else if (bms_data_acq.temperature > bms_protect_params.OTC_Protect)
        {
            BMS_ControlCharge(0);
            bms_protect_alert = FlAG_ALERT_OTC;
            bms_protect_state = PROTECT_STATE_RELIEVE_WAIT;
            LOG_W("OTC Fault\r\n");
        }
        // 充电低温保护
        else if (bms_data_acq.temperature < bms_protect_params.LTC_Protect)
        {
            BMS_ControlCharge(0);
            bms_protect_alert = FlAG_ALERT_LTC;
            bms_protect_state = PROTECT_STATE_RELIEVE_WAIT;
            LOG_W("LTC Fault\r\n");
        }
        break;
    case BMS_STATE_DISCHARGING:
        // 放电过温保护
        if (bms_data_acq.temperature > bms_protect_params.OTD_Protect)
        {
            BMS_ControlCharge(0);
            bms_protect_alert = FlAG_ALERT_OTD;
            bms_protect_state = PROTECT_STATE_RELIEVE_WAIT;
            LOG_W("OTD Fault\r\n");
        }
        // 放电低温保护
        else if (bms_data_acq.temperature < bms_protect_params.LTD_Protect)
        {
            BMS_ControlCharge(0);
            bms_protect_alert = FlAG_ALERT_LTD;
            bms_protect_state = PROTECT_STATE_RELIEVE_WAIT;
            LOG_W("LTD Fault\r\n");
        }
        break;
    }
}

static void BMS_ProtectHWMonitor(void)
{
    uint8_t reg_value = 0, write_value = 0;
    uint8_t ret = 0;
    ret = BQ76940_GetSystemStatus(&reg_value);
    if (ret == 0)
    {
        bms_protect_alert = FlAG_ALERT_AFE_COMM; // 通信故障
        LOG_W("AFE_COMM Fault\r\n");
        return;
    }

    if ((reg_value & 0x3F) == 0)
    {
        return; // 无故障
    }

    // 放电过流保护
    if (reg_value & STAT_OCD_BIT)
    {
        bms_protect_alert = FlAG_ALERT_OCD;
        bms_protect_state = PROTECT_STATE_RELIEVE_WAIT;
        BMS_ProtectStartTimer(bms_protect_params.OCD_RelieveTime);
        write_value |= STAT_OCD_BIT;
        LOG_W("OCD Fault\r\n");
    }
    // 放电短路保护
    if (reg_value & STAT_SCD_BIT)
    {
        bms_protect_alert = FlAG_ALERT_SCD;
        bms_protect_state = PROTECT_STATE_RELIEVE_WAIT;
        BMS_ProtectStartTimer(bms_protect_params.SCD_RelieveTime);
        write_value |= STAT_SCD_BIT;
        LOG_W("SCD Fault\r\n");
    }
    // 过压保护
    if (reg_value & STAT_OV_BIT)
    {
        bms_protect_alert = FlAG_ALERT_OV;
        bms_protect_state = PROTECT_STATE_RELIEVE_WAIT;
        write_value |= STAT_OV_BIT;
        LOG_W("OV Fault\r\n");
    }
    // 欠压保护
    if (reg_value & STAT_UV_BIT)
    {
        bms_protect_alert = FlAG_ALERT_UV;
        bms_protect_state = PROTECT_STATE_RELIEVE_WAIT;
        write_value |= STAT_UV_BIT;
        LOG_W("UV Fault\r\n");
    }
    // ALERT引脚被触发
    if (reg_value & STAT_OVRD_ALERT_BIT)
    {
        write_value |= STAT_OVRD_ALERT_BIT;
        LOG_W("OVRD_ALERT Fault\r\n");
    }
    // 芯片故障
    if (reg_value & STAT_DEVICE_XREADY_BIT)
    {
        write_value |= STAT_DEVICE_XREADY_BIT;
        LOG_W("DEVICE_XREADY Fault\r\n");
    }

    LOG_I("SYS_STAT: 0x%02X\r\n", reg_value);
    // 清除故障
    BQ76940_ClearFaults(write_value);
    LOG_I("Clear Faults: 0x%02X\r\n", write_value);

    // 获取系统控制2寄存器值
    BQ76940_GetSystemControl2(&reg_value);
    LOG_I("SYS_CTRL2: 0x%02X\r\n", reg_value);

    // 更新CHG、DSG状态
    BMS_UpdateState_CHGAndDSG(((reg_value & 0x01) != 0), ((reg_value & 0x02) != 0));
}

static void BMS_ProtectRelieveWait(void)
{
    switch (bms_protect_alert)
    {
        // 充电过压解除保护判断
    case FlAG_ALERT_OV:
        if (bms_data_acq.max_voltage < bms_protect_params.OV_Relieve)
        {
            bms_protect_state = PROTECT_STATE_RELIEVE;
            LOG_I("OV Relieve\r\n");
        }
        break;
        // 放电欠压解除保护判断
    case FlAG_ALERT_UV:
        if (bms_data_acq.min_voltage > bms_protect_params.UV_Relieve)
        {
            bms_protect_state = PROTECT_STATE_RELIEVE;
            LOG_I("UV Relieve\r\n");
        }
        break;
        // 充电过温解除保护判断
    case FlAG_ALERT_OTC:
        if (bms_data_acq.temperature < bms_protect_params.OTC_Relieve)
        {
            bms_protect_state = PROTECT_STATE_RELIEVE;
            LOG_I("OTC Relieve\r\n");
        }
        break;
        // 充电低温解除保护判断
    case FlAG_ALERT_LTC:
        if (bms_data_acq.temperature > bms_protect_params.LTC_Relieve)
        {
            bms_protect_state = PROTECT_STATE_RELIEVE;
            LOG_I("LTC Relieve\r\n");
        }
        break;
        // 放电过温解除保护判断
    case FlAG_ALERT_OTD:
        if (bms_data_acq.temperature < bms_protect_params.OTD_Relieve)
        {
            bms_protect_state = PROTECT_STATE_RELIEVE;
            LOG_I("OTD Relieve\r\n");
        }
        break;
        // 放电低温解除保护判断
    case FlAG_ALERT_LTD:
        if (bms_data_acq.temperature > bms_protect_params.LTD_Relieve)
        {
            bms_protect_state = PROTECT_STATE_RELIEVE;
            LOG_I("LTD Relieve\r\n");
        }
        break;

    default:
        break;
    }
}

// 执行解除保护
static void BMS_ProtectRelieve(void)
{
    // 过压、欠压不解除保护
    switch (bms_protect_alert)
    {
    case FlAG_ALERT_OCC:
    case FlAG_ALERT_OTC:
    case FlAG_ALERT_LTC:
        BMS_ControlCharge(1);
        break;

    case FlAG_ALERT_OCD:
    case FlAG_ALERT_SCD:
    case FlAG_ALERT_OTD:
    case FlAG_ALERT_LTD:
        BMS_ControlDischarge(1);
    }

    // 解除保护后，重置保护状态和警报标志
    bms_protect_alert = FLAG_ALERT_NONE;
    bms_protect_state = PROTECT_STATE_MONITOR;
    LOG_I("Fault Protect Relieve\r\n");
}

// 用于保护任务的定时器回调入口
static void BMS_ProtectTimerEntry(void *paramter)
{
    bms_protect_state = PROTECT_STATE_RELIEVE;

    LOG_I("Protect Timer Tigger\r\n");
}

// 启动用户保护任务的定时器
static void BMS_ProtectStartTimer(uint32_t sec)
{
    uint32_t tick;

    tick = sec * 1000;
    osTimerStart(pTimerProtect, tick);

    LOG_I("Protect Timer Start\r\n");
}
