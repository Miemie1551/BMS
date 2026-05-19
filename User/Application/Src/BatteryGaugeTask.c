#include "BatteryGaugeTask.h"
#include "cmsis_os.h"

#include "DataAcqTask.h"
#include "StateControlTask.h"

#define BATTERY_GAUGE_TASK_PERIOD_MS 1000

BMS_BatteryGaugeData_t bms_battery_gauge_data =
    {
        .soc = 500,
        .capacity_rated = 0,
        .capacity_real = 0,
        .capacity_remain = 0,
};

// 三元锂电池 SOC开路电压法计算数据表
// 支持磷酸铁锂、钛酸锂也得做一张这个表
const uint16_t SocOcvTab[101] = {
    3282,                                                       // 0%~1%
    3309, 3334, 3357, 3378, 3398, 3417, 3434, 3449, 3464, 3477, // 1%~10%
    3489, 3500, 3510, 3520, 3528, 3536, 3543, 3549, 3555, 3561, // 11%~20%
    3566, 3571, 3575, 3579, 3583, 3586, 3590, 3593, 3596, 3599, // 21%~30%
    3602, 3605, 3608, 3611, 3615, 3618, 3621, 3624, 3628, 3632, // 31%~40%
    3636, 3640, 3644, 3648, 3653, 3658, 3663, 3668, 3674, 3679, // 41%~50%
    3685, 3691, 3698, 3704, 3711, 3718, 3725, 3733, 3741, 3748, // 51%~60%
    3756, 3765, 3773, 3782, 3791, 3800, 3809, 3818, 3827, 3837, // 61%~70%
    3847, 3857, 3867, 3877, 3887, 3897, 3908, 3919, 3929, 3940, // 71%~80%
    3951, 3962, 3973, 3985, 3996, 4008, 4019, 4031, 4043, 4055, // 81%~90%
    4067, 4080, 4092, 4105, 4118, 4131, 4145, 4158, 4172, 4185, // 91~100%
};

static void BMS_OcvCalculateToSoc(uint16_t voltage);
static int16_t right_bound(const uint16_t *nums, uint8_t start_pos, uint8_t end_pos, uint16_t target);

// SOC/SOH计算、容量学习
void BatteryGaugeTask(void *argument)
{

    for (;;)
    {
        switch (bms_sys_state)
        {
        case BMS_STATE_STANDBY:
        case BMS_STATE_FAULT:
            // 开路电压法计算SOC
            BMS_OcvCalculateToSoc(bms_data_acq.avg_voltage);
            break;

        case BMS_STATE_CHARGING:
        case BMS_STATE_DISCHARGING:
            // 安时积分法计算SOC(未实现)
            BMS_OcvCalculateToSoc(bms_data_acq.avg_voltage);
            break;
        default:
            break;
        }
        osDelay(BATTERY_GAUGE_TASK_PERIOD_MS);
    }
}

// 根据单体电芯最低电压计算出soc值,用于上电和长时间静止状态下的校准
static void BMS_OcvCalculateToSoc(uint16_t voltage)
{
    uint16_t soc = 0;

    if (voltage <= SocOcvTab[0])
    {
        soc = 0;
    }
    else if (voltage >= SocOcvTab[100])
    {
        soc = 1000;
    }
    else
    {
        uint16_t index = right_bound(SocOcvTab, 0, 100, voltage);

        if (voltage == SocOcvTab[index])
        {
            // 整数SOC值
            soc = index * 10;
        }
        else
        {
            // 计算百分比后的小数点
            soc = index * 10 + ((SocOcvTab[index] - voltage) * 10) / (SocOcvTab[index] - SocOcvTab[index + 1]);
        }
    }

    bms_battery_gauge_data.soc = soc;
}

// 查找一个数在数组中的右侧边界(二分法)
// start_pos：起始位置
// end_pos：结束位置
// 返回-1：表示不存在这个数
static int16_t right_bound(const uint16_t *nums, uint8_t start_pos, uint8_t end_pos, uint16_t target)
{
    uint16_t left = start_pos;
    uint16_t right = end_pos;

    while (left < right)
    {
        int mid = (left + right) / 2;

        if (nums[mid] == target)
        {
            left = mid + 1; // 注意
        }
        else if (nums[mid] < target)
        {
            left = mid + 1;
        }
        else if (nums[mid] > target)
        {
            right = mid;
        }
    }
    if ((left - 1) < start_pos)
    {
        return -1;
    }
    return left - 1; // 注意
}
