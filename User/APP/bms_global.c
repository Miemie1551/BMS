#include "bms_global.h"

// 全局BMS信息结构体变量
BMS_Info_t g_bms_info = {0};

// 互斥锁（保护g_bms_info）
osMutexId_t g_bms_info_mutex;

/**
 * @brief BMS全局初始化函数
 * */
void BMS_Global_Init(void)
{
    // 创建互斥锁
    osMutexAttr_t bms_info_mutex_attr = {
        .name = "BMSInfoMutex",
    };
    g_bms_info_mutex = osMutexNew(&bms_info_mutex_attr);
}

/**
 * @brief 线程安全地读取BMS信息
 * @param dest 指向目标BMS_Info_t结构体的指针
 */
void BMS_Info_Read(BMS_Info_t *dest)
{
    if (g_bms_info_mutex != NULL && dest != NULL)
    {
        osMutexAcquire(g_bms_info_mutex, osWaitForever);
        *dest = g_bms_info;
        osMutexRelease(g_bms_info_mutex);
    }
}

/**
 * @brief 线程安全地写入BMS信息
 * @param src 指向源BMS_Info_t结构体的指针
 */
void BMS_Info_Write(const BMS_Info_t *src)
{
    if (g_bms_info_mutex != NULL && src != NULL)
    {
        osMutexAcquire(g_bms_info_mutex, osWaitForever);
        g_bms_info = *src;
        osMutexRelease(g_bms_info_mutex);
    }
}
