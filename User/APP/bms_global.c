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
    // 最低位区分递归互斥锁和普通互斥锁，0为普通互斥锁，1为递归互斥锁
}

/**
 * @brief 复制BMS信息至目标内存地址
 * @param dest 指向目标BMS_Info_t结构体的指针
 */
void BMS_CopyBMSInfo(BMS_Info_t *dest)
{
    if (dest != NULL)
    {
        *dest = bms_info;
    }
}

/**
 * @brief 获取BMS信息指针至目标内存地址
 * @param dest 指向目标BMS_Info_t结构体的指针
 */
void BMS_GetBMSInfoPtr(BMS_Info_t *dest)
{
    if (&bms_info != NULL)
    {
        dest = &bms_info;
    }
}

/**
 * @brief 线程安全地获取BMS信息互斥锁
 * @note 调用者必须在获取互斥锁后调用BMS_ReleaseBMSInfoMutex()释放互斥锁
 */
void BMS_AcquireBMSInfoMutex(void)
{
    if (bms_info_mutex != NULL)
    {
        osMutexAcquire(bms_info_mutex, pdMS_TO_TICKS(1000));
    }
}

/**
 * @brief 线程安全地释放BMS信息互斥锁
 * @note 调用者必须在获取互斥锁后调用此函数释放互斥锁
 */
void BMS_ReleaseBMSInfoMutex(void)
{
    if (bms_info_mutex != NULL)
    {
        osMutexRelease(bms_info_mutex);
    }
}
