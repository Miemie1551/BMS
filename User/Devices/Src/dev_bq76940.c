#include "dev_bq76940.h"
#include "math.h"

#include "bsp_soft_i2c.h"
#include "bsp_usart.h"

/********************************* BQ769X0 DEBUG **************************/
#define PRINTF Printf

#define BQ76940_DEBUG_LEVEL 1 // 日志级别

#if (BQ76940_DEBUG_LEVEL == 0)

#define LOG_E(...) \
    do             \
    {              \
    } while (0)
#define LOG_D(...) \
    do             \
    {              \
    } while (0)
#define LOG_I(...) \
    do             \
    {              \
    } while (0)

#elif (BQ76940_DEBUG_LEVEL == 1)

#define LOG_E(fmt, arg...) PRINTF("[BQ76940] [ERROR]" fmt, ##arg)
#define LOG_D(fmt, arg...) PRINTF("[BQ76940] [DEBUG]" fmt, ##arg)
#define LOG_I(fmt, arg...) PRINTF("[BQ76940] [INFO]" fmt, ##arg)

#elif (BQ76940_DEBUG_LEVEL == 2)

#define LOG_E(fmt, arg...) PRINTF("[BQ76940] [ERROR][%s:%s:%d] ", __FILE__, __FUNCTION__, __LINE__, fmt, ##arg)
#define LOG_D(fmt, arg...) PRINTF("[BQ76940] [DEBUG][%s:%s:%d] ", __FILE__, __FUNCTION__, __LINE__, fmt, ##arg)
#define LOG_I(fmt, arg...) PRINTF("[BQ76940] [INFO][%s:%s:%d] ", __FILE__, __FUNCTION__, __LINE__, fmt, ##arg)
#endif
/********************************************************************************/

/* GPIO 引脚定义 */
#define BQ76940_I2C_SCL_PIN GPIO_PIN_8
#define BQ76940_I2C_SCL_PORT GPIOB
#define BQ76940_I2C_SDA_PIN GPIO_PIN_9
#define BQ76940_I2C_SDA_PORT GPIOB
#define BQ76940_WAKE_PIN GPIO_PIN_8
#define BQ76940_WAKE_PORT GPIOA

/* BQ7694003DBTR 设备地址 */
#define BQ76940_DEVICE_ADDR 0x08

/* BQ76940 寄存器地址定义 **************************************************/
/* 寄存器地址定义 */
#define SYS_STAT 0x00
#define CELLBAL1 0x01
#define CELLBAL2 0x02
#define CELLBAL3 0x03
#define SYS_CTRL1 0x04
#define SYS_CTRL2 0x05
#define PROTECT1 0x06
#define PROTECT2 0x07
#define PROTECT3 0x08
#define OV_TRIP 0x09
#define UV_TRIP 0x0A
#define CC_CFG 0x0B
/* 电池电压寄存器 */
#define VC1_HI 0x0C
#define VC1_LO 0x0D
#define VC2_HI 0x0E
#define VC2_LO 0x0F
#define VC3_HI 0x10
#define VC3_LO 0x11
#define VC4_HI 0x12
#define VC4_LO 0x13
#define VC5_HI 0x14
#define VC5_LO 0x15
#define VC6_HI 0x16
#define VC6_LO 0x17
#define VC7_HI 0x18
#define VC7_LO 0x19
#define VC8_HI 0x1A
#define VC8_LO 0x1B
#define VC9_HI 0x1C
#define VC9_LO 0x1D
#define VC10_HI 0x1E
#define VC10_LO 0x1F
#define VC11_HI 0x20
#define VC11_LO 0x21
#define VC12_HI 0x22
#define VC12_LO 0x23
#define VC13_HI 0x24
#define VC13_LO 0x25
#define VC14_HI 0x26
#define VC14_LO 0x27
#define VC15_HI 0x28
#define VC15_LO 0x29
/* 总电压、温度、电流寄存器 */
#define BAT_HI 0x2A
#define BAT_LO 0x2B
#define TS1_HI 0x2C
#define TS1_LO 0x2D
#define TS2_HI 0x2E
#define TS2_LO 0x2F
#define TS3_HI 0x30
#define TS3_LO 0x31
#define CC_HI 0x32
#define CC_LO 0x33
/* 校准寄存器 */
#define ADCGAIN1 0x50
#define ADCOFFSET 0x51
#define ADCGAIN2 0x59
/* END ***********************************************************************/

// DSG和CHG控制位定义
#define DSG_ON 0x01 << 1
#define CHG_ON 0x01 << 0

// 过压和欠压保护阈值（单位：mV）
#define OV_THRESHOLD 4200 // OV范围：3.15~4.70V     3136~4674mV
#define UV_THRESHOLD 3100 // UV范围：1.58~3.10V     1589~3125mV

// 校准参数
static uint16_t adc_gain = 0; // 377uV
static int8_t adc_offset = 0; // 45mV

// I2C 句柄定义
static I2C_Handle_t hi2c = {
    .I2C_Port = BQ76940_I2C_SCL_PORT,
    .SCL_Pin = BQ76940_I2C_SCL_PIN,
    .SDA_Pin = BQ76940_I2C_SDA_PIN,
    .DeviceAddress = BQ76940_DEVICE_ADDR};

// 电芯id索引映射表
const uint8_t cell_id_index_map[CELL_TOTAL] = {
    1, 2,
    5, 6, 7,
    10, 11, 12,
    15};

// 私有函数声明
static void BQ76940_GPIOInit(void);
static uint8_t CRC8_Calculate(uint8_t *data, uint16_t length);
static uint8_t BQ76940_WriteByteWithCRC(uint8_t reg_addr, uint8_t data);
static uint8_t BQ76940_ReadByteWithCRC(uint8_t reg_addr, uint8_t *data);
static uint8_t BQ76940_ReadHalfWordWithCRC(uint8_t reg_addr, uint16_t *data);

static uint8_t BQ76940_GetCalibrationParams(uint16_t *adc_gain, int8_t *adc_offset);
static void BQ76940_ConfigureTRIP(void);

/**
 *  @brief 初始化BQ76940芯片
 *  @return 初始化结果（1表示成功，0表示失败）
 */
uint8_t BQ76940_Init(void)
{
    BQ76940_GPIOInit();

    /* 1. 唤醒芯片 */
    HAL_GPIO_WritePin(BQ76940_WAKE_PORT, BQ76940_WAKE_PIN, GPIO_PIN_SET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(BQ76940_WAKE_PORT, BQ76940_WAKE_PIN, GPIO_PIN_RESET);

    /* 2. 等待芯片启动完成 */
    HAL_Delay(10);

    /* 3. 获取校准参数：ADC Gain = 377uV, ADC Offset = 45mV */
    if (BQ76940_GetCalibrationParams(&adc_gain, &adc_offset) != 1)
    {
        LOG_E("Failed to read calibration parameters\r\n");
        return 0;
    }
    // LOG_I("Calibration parameters: ADC Gain = %duV, ADC Offset = %dmV\r\n", adc_gain, adc_offset);

    /* 4. 配置SYS_CTRL1：启用ADC（及OV/UV保护）、外部热敏电阻 */
    BQ76940_WriteByteWithCRC(SYS_CTRL1, 0x18); // 0x18 = 00011000b

    /* 5. 配置SYS_CTRL2：启用CC（连续读取模式），打开DSG、CHG */
    BQ76940_WriteByteWithCRC(SYS_CTRL2, 0x43); // 0x43 = 01000011b

    /* 6. 配置PROTECT1：低量程，放电短路延迟：400us，短路电压：22mV。换算短路电流：22mV/4mΩ=5.5A */
    BQ76940_WriteByteWithCRC(PROTECT1, 0x18); // 0x18 = 0 00 11 000b

    /* 7. 配置PROTECT2：放电过流延迟：320ms，过流电压：11mV。换算过流电流：11mV/4mΩ=2.75A */
    BQ76940_WriteByteWithCRC(PROTECT2, 0x71); // 0x71 = 0 101 0001b

    /* 8. 配置PROTECT3：欠压延迟：4s，过压延迟：4s */
    BQ76940_WriteByteWithCRC(PROTECT3, 0x60); // 0x60 = 01 10 0000b

    /* 9. 配置过压和欠压保护阈值：OV_THRESHOLD=4200mV，UV_THRESHOLD=3100mV */
    BQ76940_ConfigureTRIP();

    /* 10. 配置CC_CFG：默认最优配置 */
    BQ76940_WriteByteWithCRC(CC_CFG, 0x19);

    /* 11. 清除故障状态 */
    BQ76940_WriteByteWithCRC(SYS_STAT, 0xFF);

    LOG_I("BQ76940 initialization completed successfully\r\n");
    return 1;
}

/**
 * @brief 初始化BQ76940的GPIO引脚
 *
 * @note SCL和SDA引脚配置为开漏输出，初始状态拉高；WAKE引脚配置为推挽输出，初始状态拉低
 */
static void BQ76940_GPIOInit(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pin = BQ76940_I2C_SCL_PIN | BQ76940_I2C_SDA_PIN;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(BQ76940_I2C_SCL_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(BQ76940_I2C_SCL_PORT, BQ76940_I2C_SCL_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(BQ76940_I2C_SDA_PORT, BQ76940_I2C_SDA_PIN, GPIO_PIN_SET);

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pin = BQ76940_WAKE_PIN;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(BQ76940_WAKE_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(BQ76940_WAKE_PORT, BQ76940_WAKE_PIN, GPIO_PIN_RESET);
}

/**
 *  @brief 计算CRC8校验值
 *
 *  @note CRC8算法采用多项式0x07，初始值0x00，输入数据不反转，输出CRC不反转
 *
 *  @param[in] data 待校验数据指针
 *  @param length 待校验数据长度
 *  @return CRC8校验值
 */
static uint8_t CRC8_Calculate(uint8_t *data, uint16_t length)
{
    uint8_t crc = 0x00;
    for (uint16_t i = 0; i < length; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x07;
            else
                crc <<= 1;
        }
    }
    return crc;
}

/**
 *  @brief 向BQ76940写入一个字节数据（包含CRC校验）
 *  @param reg_addr 寄存器地址
 *  @param data 待写入的数据
 * @return 写入结果（1表示成功，0表示失败）
 */
static uint8_t BQ76940_WriteByteWithCRC(uint8_t reg_addr, uint8_t data)
{
    uint8_t crc_buf[3] = {0};
    uint8_t send_buf[2] = {0};
    uint8_t crc = 0;

    crc_buf[0] = BQ76940_DEVICE_ADDR << 1; // Write operation
    crc_buf[1] = reg_addr;
    crc_buf[2] = data;

    crc = CRC8_Calculate(crc_buf, 3);
    send_buf[0] = data;
    send_buf[1] = crc;

    if (I2C_Write(&hi2c, reg_addr, send_buf, 2) != 1)
    {
        LOG_E("Failed to write I2C data\r\n");
        return 0;
    }
    return 1;
}

/**
 * @brief 从BQ76940读取一个字节数据（包含CRC校验）
 * @param reg_addr 寄存器地址
 * @param data 读取到的数据指针
 * @return 读取结果（1表示成功，0表示失败）
 */
static uint8_t BQ76940_ReadByteWithCRC(uint8_t reg_addr, uint8_t *data)
{
    uint8_t crc_buf[2] = {0};
    uint8_t crc;
    uint8_t recv_data[2] = {0};

    if (I2C_Read(&hi2c, reg_addr, recv_data, 2) != 1)
    {
        LOG_E("Failed to read I2C data\r\n");
        return 0; // Return an invalid value to indicate failure
    }
    // CRC校验：先构造CRC输入数据（设备地址 + 读位 + 读到的数据）
    crc_buf[0] = (BQ76940_DEVICE_ADDR << 1) | 0x01; // Read operation
    crc_buf[1] = recv_data[0];                      // Received data
    crc = CRC8_Calculate(crc_buf, 2);
    // LOG_D("Received byte: 0x%02X, CRC from device: 0x%02X, CRC calculated: 0x%02X\r\n", recv_data[0], recv_data[1], crc);
    if (crc != recv_data[1])
    {
        LOG_E("CRC check failed! Expected: 0x%02X, Received: 0x%02X\r\n", crc, recv_data[1]);
        // return 0;
    }

    *data = recv_data[0];
    return 1; // Success
}

/**
 * @brief 从BQ76940读取一个半字数据（包含CRC校验）
 * @param reg_addr 寄存器地址
 * @param data 读取到的数据指针
 * @return 读取结果（1表示成功，0表示失败）
 */
static uint8_t BQ76940_ReadHalfWordWithCRC(uint8_t reg_addr, uint16_t *data)
{
    uint8_t crc_buf[2] = {0};
    uint8_t crc;
    uint8_t recv_data[4] = {0}; // 1字节数据 + 1字节CRC + 1字节数据 + 1字节CRC

    if (I2C_Read(&hi2c, reg_addr, recv_data, 4) != 1)
    {
        LOG_E("Failed to read I2C data\r\n");
        return 0; // Return an invalid value to indicate failure
    }

    // 第一个字节CRC校验：先构造CRC输入数据（设备地址 + 读位 + 读到的数据）
    crc_buf[0] = (BQ76940_DEVICE_ADDR << 1) | 0x01; // Read operation
    crc_buf[1] = recv_data[0];                      // Received data
    crc = CRC8_Calculate(crc_buf, 2);
    // LOG_D("Received byte: 0x%02X, CRC from device: 0x%02X, CRC calculated: 0x%02X\r\n", recv_data[0], recv_data[1], crc);
    if (crc != recv_data[1])
    {
        LOG_E("CRC check failed! Expected: 0x%02X, Received: 0x%02X\r\n", crc, recv_data[1]);
        // return 0;
    }

    // 第二个字节CRC校验：先构造CRC输入数据（读到的数据）
    crc_buf[0] = recv_data[2]; // Received data
    crc = CRC8_Calculate(crc_buf, 1);
    // LOG_D("Received byte: 0x%02X, CRC from device: 0x%02X, CRC calculated: 0x%02X\r\n", recv_data[2], recv_data[3], crc);
    if (crc != recv_data[3])
    {
        LOG_E("CRC check failed! Expected: 0x%02X, Received: 0x%02X\r\n", crc, recv_data[3]);
        // return 0;
    }

    *data = ((recv_data[0] << 8) | recv_data[2]);
    return 1; // Success
}

/**
 * @brief 获取BQ76940校准参数
 * @param adc_gain ADC增益指针
 * @param adc_offset ADC偏置指针
 * @return 获取结果（1表示成功，0表示失败）
 */
static uint8_t BQ76940_GetCalibrationParams(uint16_t *adc_gain, int8_t *adc_offset)
{
    uint8_t gain1 = 0, gain2 = 0;
    uint8_t offset = 0;

    if (BQ76940_ReadByteWithCRC(ADCGAIN1, &gain1) != 1)
    {
        LOG_E("Failed to read ADCGAIN1\r\n");
        return 0;
    }

    if (BQ76940_ReadByteWithCRC(ADCGAIN2, &gain2) != 1)
    {
        LOG_E("Failed to read ADCGAIN2\r\n");
        return 0;
    }

    if (BQ76940_ReadByteWithCRC(ADCOFFSET, &offset) != 1)
    {
        LOG_E("Failed to read ADCOFFSET\r\n");
        return 0;
    }

    *adc_gain = (((gain1 & 0x0C) << 1) | ((gain2 & 0xE0) >> 5)) + 365;
    *adc_offset = (int8_t)offset;
    return 1;
}

/**
 * @brief 配置BQ76940的过压和欠压保护阈值
 * @note 根据ADC增益和偏置计算TRIP阈值，并写入对应寄存器
 */
static void BQ76940_ConfigureTRIP(void)
{
    uint16_t ov_trip_full = 0, uv_trip_full = 0;
    uint8_t ov_trip = 0, uv_trip = 0;

    ov_trip_full = (OV_THRESHOLD - adc_offset) / (adc_gain * 0.001f);
    uv_trip_full = (UV_THRESHOLD - adc_offset) / (adc_gain * 0.001f);

    ov_trip = (ov_trip_full >> 4) & 0xFF;
    uv_trip = (uv_trip_full >> 4) & 0xFF;

    BQ76940_WriteByteWithCRC(OV_TRIP, ov_trip);
    BQ76940_WriteByteWithCRC(UV_TRIP, uv_trip);
}

/**
 * @brief 获取系统状态寄存器的值
 * @param status_bits 存储系统状态的指针
 * @return 获取结果（1表示成功，0表示失败）
 */
uint8_t BQ76940_GetSystemStatus(uint8_t *status_bits)
{
    return BQ76940_ReadByteWithCRC(SYS_STAT, status_bits);
}

/**
 * @brief 清除故障状态
 * @param bit_mask 要清除的故障位掩码
 * @return 清除结果（1表示成功，0表示失败）
 */
uint8_t BQ76940_ClearFaults(uint8_t bit_mask)
{
    return BQ76940_WriteByteWithCRC(SYS_STAT, bit_mask);
}

/**
 * @brief 获取系统控制2寄存器的值
 * @param ctrl2_bits 存储系统控制2寄存器值的指针
 * @return 获取结果（1表示成功，0表示失败）
 */
uint8_t BQ76940_GetSystemControl2(uint8_t *ctrl2_bits)
{
    return BQ76940_ReadByteWithCRC(SYS_CTRL2, ctrl2_bits);
}

/**
 * @brief 启用放电
 * @return 操作结果（1表示成功，0表示失败）
 */
uint8_t BQ76940_EnableDischarging(void)
{
    uint8_t sys_ctrl2_val = 0;
    if (BQ76940_ReadByteWithCRC(SYS_CTRL2, &sys_ctrl2_val) != 1)
    {
        return 0;
    }
    return BQ76940_WriteByteWithCRC(SYS_CTRL2, sys_ctrl2_val | DSG_ON);
}

/**
 * @brief 禁用放电
 * @return 操作结果（1表示成功，0表示失败）
 */
uint8_t BQ76940_DisableDischarging(void)
{
    uint8_t sys_ctrl2_val = 0;
    if (BQ76940_ReadByteWithCRC(SYS_CTRL2, &sys_ctrl2_val) != 1)
    {
        return 0;
    }
    return BQ76940_WriteByteWithCRC(SYS_CTRL2, sys_ctrl2_val & ~DSG_ON);
}

/**
 * @brief 启用充电
 * @return 操作结果（1表示成功，0表示失败）
 */
uint8_t BQ76940_EnableCharging(void)
{
    uint8_t sys_ctrl2_val = 0;
    if (BQ76940_ReadByteWithCRC(SYS_CTRL2, &sys_ctrl2_val) != 1)
    {
        return 0;
    }
    return BQ76940_WriteByteWithCRC(SYS_CTRL2, sys_ctrl2_val | CHG_ON);
}

/**
 * @brief 禁用充电
 * @return 操作结果（1表示成功，0表示失败）
 */
uint8_t BQ76940_DisableCharging(void)
{
    uint8_t sys_ctrl2_val = 0;
    if (BQ76940_ReadByteWithCRC(SYS_CTRL2, &sys_ctrl2_val) != 1)
    {
        return 0;
    }
    return BQ76940_WriteByteWithCRC(SYS_CTRL2, sys_ctrl2_val & ~CHG_ON);
}

/**
 * @brief 读取所有电池单体电压
 * @param voltages 存储电压值的数组指针，长度至少为9
 * @param total_voltage 存储总电压值的指针
 * @return 读取结果（1表示成功，0表示失败）
 */
uint8_t BQ76940_ReadCellVoltages(uint16_t _voltages[])
{
    uint8_t cell_index = 0;
    uint16_t voltage_val = 0;

    for (uint8_t i = 0; i < CELL_TOTAL; i++)
    {
        cell_index = cell_id_index_map[i] - 1;
        if (BQ76940_ReadHalfWordWithCRC(VC1_HI + (cell_index * 2), &voltage_val) == 1)
        {
            _voltages[i] = (voltage_val * adc_gain) * 0.001f + adc_offset; // 转换为实际电压值
        }
        else
        {
            LOG_E("Failed to read voltage for cell %d\r\n", i + 1);
            return 0; // Return failure if any cell voltage read fails
        }
    }

    return 1; // Success
}

/**
 * @brief 读取所有电池单体电压
 * @param voltages 存储电压值的数组指针，长度至少为9
 * @param total_voltage 存储总电压值的指针
 * @return 读取结果（1表示成功，0表示失败）
 */
uint8_t BQ76940_ReadBatteryVoltage(uint16_t *_voltage)
{
    uint16_t voltage_val = 0;

    if (BQ76940_ReadHalfWordWithCRC(BAT_HI, &voltage_val) == 1)
    {
        *_voltage = 4 * adc_gain * voltage_val * 0.001f + (CELL_TOTAL * adc_offset);
    }
    else
    {
        LOG_E("Failed to read battery voltage\r\n");
        return 0; // Return failure if any cell voltage read fails
    }

    return 1; // Success
}

/**
 * @brief 读取电流值
 * @param current 存储电流值的指针，单位mA
 * @return 读取结果（1表示成功，0表示失败）
 */
uint8_t BQ76940_ReadCurrent(int16_t *_current)
{
    int16_t cc_adc_val = 0; // 库仑计数寄存器的原始ADC值
    float cc_val = 0;       // 转换后的库仑计数值

    if (BQ76940_ReadHalfWordWithCRC(CC_HI, (uint16_t *)&cc_adc_val) == 1)
    {
        cc_val = cc_adc_val * 8.44f;       // 转换为库仑计数值，单位：uV
        *_current = (int16_t)(cc_val / 4); // 转换为电流值，单位：mA（采样电阻为4mΩ）
        // LOG_D("Raw ADC value: %d, CC value: %.2f uV, Current: %d mA\r\n", cc_adc_val, cc_val, *current);
        return 1; // Success
    }
    else
    {
        LOG_E("Failed to read current\r\n");
        return 0; // Return failure
    }
}

/* 热敏电阻参数 */
const float Rp = 10000;       // 热敏电阻25℃标称阻值（10kΩ）
const float T2 = 273.15 + 25; // 热敏电阻25℃的绝对温度（开尔文）
const float Bx = 3380;        // 热敏电阻的B值（单位：K）
const float Ka = 273.15;      // 开尔文与摄氏度的转换常数

/**
 * @brief 读取温度值
 * @param temperature 存储温度值的指针，单位℃
 * @retval 读取结果（1表示成功，0表示失败）
 */
uint8_t BQ76940_ReadTemperature(int16_t *_temperature)
{
    uint16_t temp_adc_val = 0; // 从BQ76940读取的原始ADC值
    float V_tsx = 0;           // 热敏电阻两端的电压值
    float Rt = 0;              // 当前热敏电阻的阻值
    float temp_kelvin = 0;     // 温度的开尔文值

    if (BQ76940_ReadHalfWordWithCRC(TS1_HI, &temp_adc_val) == 1)
    {
        V_tsx = (temp_adc_val * 382) * 0.001f;
        Rt = (10000 * V_tsx) / (3300.0f - V_tsx);
        temp_kelvin = 1 / (1 / T2 + (log(Rt / Rp)) / Bx);
        *_temperature = (int16_t)((temp_kelvin - Ka + 0.5f) * 10); // +0.5 的误差矫正

        // LOG_D("Raw ADC value: %d, V_tsx: %.2f mV, Rt: %.2f ohms, TempKelvin: %.2f, Temperature: %d\r\n",
        //       temp_adc_val, V_tsx, Rt, temp_kelvin, *_temperature);
        return 1; // Success
    }
    else
    {
        LOG_E("Failed to read temperature\r\n");
        return 0; // Return failure
    }
}

/**
 * @brief 开始均衡指定电芯
 * @param _cell_id 电芯ID（1-15)
 * @retval None
 */
void BQ76940_StartBalancing(uint8_t _cell_id)
{
    if (_cell_id < 1 || _cell_id > 15)
    {
        LOG_E("Invalid cell ID for balancing: %d\r\n", _cell_id);
        return;
    }

    uint8_t bal_reg_addr = 0;
    uint8_t bal_bit_mask = 0;

    // 根据cell_id计算对应的均衡寄存器地址和位掩码
    if (_cell_id <= 5)
    {
        bal_reg_addr = CELLBAL1;
        bal_bit_mask = 1 << (_cell_id - 1);
    }
    else if (_cell_id <= 10)
    {
        bal_reg_addr = CELLBAL2;
        bal_bit_mask = 1 << (_cell_id - 6);
    }
    else
    {
        bal_reg_addr = CELLBAL3;
        bal_bit_mask = 1 << (_cell_id - 11);
    }

    LOG_D("Balancing cell %d: Write 0x%02X to register 0x%02X\r\n", _cell_id, bal_bit_mask, bal_reg_addr);
    // 启动均衡
    BQ76940_WriteByteWithCRC(bal_reg_addr, bal_bit_mask);
}

/**
 * @brief 停止均衡指定电芯
 * @param _cell_id 电芯ID（1-15)
 * @retval None
 */
void BQ76940_StopBalancing(uint8_t _cell_id)
{
    if (_cell_id < 1 || _cell_id > 15)
    {
        LOG_E("Invalid cell ID for balancing: %d\r\n", _cell_id);
        return;
    }

    // 根据cell_id计算对应的均衡寄存器地址和位掩码
    if (_cell_id <= 5)
    {
        BQ76940_WriteByteWithCRC(CELLBAL1, 0);
    }
    else if (_cell_id <= 10)
    {
        BQ76940_WriteByteWithCRC(CELLBAL2, 0);
    }
    else
    {
        BQ76940_WriteByteWithCRC(CELLBAL3, 0);
    }
}
