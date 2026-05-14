#include "bms_global.h"

// BMS信息结构体变量
static BMS_Info_t bms_info = {0};

// 互斥锁（保护bms_info）
static osMutexId_t bms_info_mutex;

/**
 * @brief BMS信息初始化函数
 * */
void BMS_Info_Init(void)
{
    // 创建互斥锁
    osMutexAttr_t bms_info_mutex_attr = {
        .name = "BMSInfoMutex",
    };
    bms_info_mutex = osMutexNew(&bms_info_mutex_attr);
}

/**
 * @brief 线程安全地读取BMS信息
 * @param dest 指向目标BMS_Info_t结构体的指针
 */
void BMS_Info_Read(BMS_Info_t *dest)
{
    if (bms_info_mutex != NULL && dest != NULL)
    {
        osMutexAcquire(bms_info_mutex, osWaitForever);
        *dest = bms_info;
        osMutexRelease(bms_info_mutex);
    }
}

/**
 * @brief 线程安全地写入BMS信息
 * @param src 指向源BMS_Info_t结构体的指针
 */
void BMS_Info_Write(const BMS_Info_t *src)
{
    if (bms_info_mutex != NULL && src != NULL)
    {
        osMutexAcquire(bms_info_mutex, osWaitForever);
        bms_info = *src;
        osMutexRelease(bms_info_mutex);
    }
}
