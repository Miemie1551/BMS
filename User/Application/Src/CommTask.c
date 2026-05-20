#include "CommTask.h"

#include "bsp_can.h"
#include "bsp_usart.h"

#define LOG_I(fmt, ...) Printf("[INFO] " fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...) Printf("[ERROR] " fmt, ##__VA_ARGS__)

osMessageQueueId_t canRxMQHandle = NULL;
const osMessageQueueAttr_t canRxMQHandle_attr = {
    .name = "canRxMQHandle"};

CAN_RxMsg_t can_rx_msg = {0};

// CAN/RS485通信
void CommTask(void *argument)
{
    // 初始化CAN
    BSP_CAN_Init();
    // 创建CAN接收消息队列
    canRxMQHandle = osMessageQueueNew(5, sizeof(CAN_RxMsg_t), &canRxMQHandle_attr);
    if (canRxMQHandle == NULL)
    {
        LOG_E("canMessageQueueNew failed\r\n");
    }

    for (;;)
    {
        // 接收CAN消息
        osStatus_t stat = osMessageQueueGet(canRxMQHandle, &can_rx_msg, NULL, 2000);
        // 处理CAN消息
        if (stat == osOK)
        {
            LOG_I("CAN Rx: id = 0x%04X, dlc = %d, data = ", can_rx_msg.id, can_rx_msg.dlc);

            for (uint8_t i = 0; i < can_rx_msg.dlc - 1; i++)
            {
                Printf("0x%02X ", can_rx_msg.data[i]);
            }
            Printf("0x%02X\r\n", can_rx_msg.data[7]);
        }

        uint32_t std_id = 0x123;
        uint8_t data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
        // 发送CAN消息
        BSP_CAN_SendStdMsg(std_id, data, 8);
    }
}
