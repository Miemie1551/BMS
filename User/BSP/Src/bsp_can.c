#include "bsp_can.h"
#include "string.h"
#include "cmsis_os.h"
#include "can.h"

#include "CommTask.h"

#include "bsp_usart.h"

#define CAN_HANDLE hcan

#define LOG_E(fmt, ...) Printf("[ERROR] " fmt, ##__VA_ARGS__)

void BSP_CAN_Init(void)
{
    CAN_FilterTypeDef sFilterConfig = {0};
    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(&CAN_HANDLE, &sFilterConfig) != HAL_OK)
    {
        LOG_E("HAL_CAN_ConfigFilter failed\r\n");
    }
    if (HAL_CAN_Start(&CAN_HANDLE) != HAL_OK)
    {
        LOG_E("HAL_CAN_Start failed\r\n");
    }
    if (HAL_CAN_ActivateNotification(&CAN_HANDLE, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
    {
        LOG_E("HAL_CAN_ActivateNotification failed\r\n");
    }
}

void BSP_CAN_SendStdMsg(uint32_t std_id, uint8_t *data, uint8_t len)
{
    CAN_TxHeaderTypeDef tx_header = {0};
    uint32_t tx_mailbox = 0;

    if (len > 8)
    {
        len = 8;
    }

    tx_header.StdId = std_id;
    tx_header.ExtId = 0;
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = len;
    tx_header.TransmitGlobalTime = DISABLE;

    // 发送CAN消息
    HAL_StatusTypeDef ret = HAL_CAN_AddTxMessage(&CAN_HANDLE, &tx_header, data, &tx_mailbox);
    if (ret != HAL_OK)
    {
        LOG_E("HAL_CAN_AddTxMessage failed, ret = 0x%04X\r\n", ret);
    }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef rx_header = {0};
    CAN_RxMsg_t rx_msg = {0};
    uint8_t rx_data[8] = {0};

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK)
    {
        // 处理接收消息
        rx_msg.id = rx_header.StdId;
        rx_msg.dlc = rx_header.DLC;
        memcpy(rx_msg.data, rx_data, rx_header.DLC);

        if (osMessageQueuePut(canRxMQHandle, &rx_msg, 0, 0) != osOK)
        {
            LOG_E("osMessageQueuePut failed\r\n");
        }
    }
}
