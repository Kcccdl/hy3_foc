#include "can_comm.h"
#include "bldc.h"
#include "main.h"
#include "string.h"

CAN_HandleTypeDef hcan1;  // CubeMX生成的CAN句柄，声明为extern或包含can.h
CAN_Status_t can_status = {0};

// CAN初始化
void CAN_Comm_Init(void)
{
    // 初始化CAN状态
    can_status.connected = 0;
    can_status.tx_count = 0;
    can_status.rx_count = 0;
    can_status.error_count = 0;
    can_status.last_rx_time = 0;
    
    // 配置CAN过滤器
    CAN_Filter_Config();
    
    // 启动CAN
    HAL_CAN_Start(&hcan1);
    
    // 启用接收中断
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}

// 发送CAN消息
void CAN_Send_Message(uint32_t id, uint8_t *data, uint8_t len)
{
    CAN_TxHeaderTypeDef tx_header;
    uint32_t tx_mailbox;
    
    // 配置发送头
    tx_header.StdId = id;              // 标准ID
    tx_header.ExtId = 0;              // 扩展ID (不使用)
    tx_header.IDE = CAN_ID_STD;        // 标准帧
    tx_header.RTR = CAN_RTR_DATA;      // 数据帧
    tx_header.DLC = len;               // 数据长度
    tx_header.TransmitGlobalTime = DISABLE;
    
    // 发送消息
    if(HAL_CAN_AddTxMessage(&hcan1, &tx_header, data, &tx_mailbox) == HAL_OK)
    {
        can_status.tx_count++;
    }
    else
    {
        can_status.error_count++;
    }
}

// CAN接收回调函数
void CAN_Receive_Callback(CAN_HandleTypeDef *hcan, CAN_Message_t *msg)
{
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];
    
    // 接收消息
    if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK)
    {
        can_status.rx_count++;
        can_status.last_rx_time = HAL_GetTick();
        can_status.connected = 1;
        
        // 填充消息结构体
        msg->id = rx_header.StdId;
        msg->len = rx_header.DLC;
        msg->format = (rx_header.IDE == CAN_ID_EXT) ? 1 : 0;
        for(int i = 0; i < 8; i++)
            msg->data[i] = rx_data[i];
        
        // 处理接收到的命令
        CAN_Process_Command(msg);
    }
}

// 处理CAN命令
void CAN_Process_Command(CAN_Message_t *msg)
{
    float value;
    uint8_t *data = msg->data;
    
    switch(msg->id)
    {
        case CAN_ID_COMMAND_SPEED:
            // 速度指令: 4字节浮点数
            if(msg->len >= 4)
            {
                memcpy(&value, data, 4);
                BLDC_SetSpeed(&motor, value);
            }
            break;
            
        case CAN_ID_COMMAND_CURRENT:
            // 电流指令: 4字节浮点数
            if(msg->len >= 4)
            {
                memcpy(&value, data, 4);
                BLDC_SetCurrent(&motor, value);
            }
            break;
            
        case CAN_ID_PARAM_SET:
            // 参数设置: 字节0=参数ID, 字节1-4=参数值
            if(msg->len >= 5)
            {
                uint8_t param_id = data[0];
                float param_value;
                memcpy(&param_value, &data[1], 4);
                
                // 根据param_id设置对应参数
                switch(param_id)
                {
                    case 0x01:  // PID Kp
                        motor.speed_pid.Kp = param_value;
                        break;
                    case 0x02:  // PID Ki
                        motor.speed_pid.Ki = param_value;
                        break;
                    case 0x03:  // PID Kd
                        motor.speed_pid.Kd = param_value;
                        break;
                }
            }
            break;
            
        default:
            break;
    }
}

// 发送状态反馈
void CAN_Send_Status(float speed, float current)
{
    uint8_t data[8];
    
    // 数据格式: 字节0-3=速度(RPM), 字节4-7=电流(A)
    memcpy(&data[0], &speed, 4);
    memcpy(&data[4], &current, 4);
    
    CAN_Send_Message(CAN_ID_STATUS, data, 8);
}

// 发送心跳包
void CAN_Send_Heartbeat(void)
{
    uint8_t data[1];
    data[0] = 0x01;  // 心跳标志
    
    CAN_Send_Message(CAN_ID_HEARTBEAT, data, 1);
}

// 设置CAN波特率
void CAN_Set_Baudrate(uint32_t baudrate)
{
    // CAN波特率设置需要通过重新初始化CAN外设
    // 这里只提供示例，实际需要修改CAN_InitTypeDef参数
}

// 配置CAN过滤器
void CAN_Filter_Config(void)
{
    CAN_FilterTypeDef can_filter;
    
    // 配置过滤器: 接收所有标准帧
    can_filter.FilterBank = 0;                          // 过滤器组0
    can_filter.FilterMode = CAN_FILTERMODE_IDMASK;       // 掩码模式
    can_filter.FilterScale = CAN_FILTERSCALE_32BIT;      // 32位
    can_filter.FilterIdHigh = 0x0000;                    // ID高位
    can_filter.FilterIdLow = 0x0000;                     // ID低位
    can_filter.FilterMaskIdHigh = 0x0000;                // 掩码高位
    can_filter.FilterMaskIdLow = 0x0000;                 // 掩码低位 (接收所有)
    can_filter.FilterFIFOAssignment = CAN_RX_FIFO0;      // 分配到FIFO0
    can_filter.FilterActivation = ENABLE;                // 激活过滤器
    can_filter.SlaveStartFilterBank = 14;                // 从过滤器组14开始
    
    HAL_CAN_ConfigFilter(&hcan1, &can_filter);
}

// CAN接收中断回调 (HAL库回调函数)
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_Message_t msg;
    if(hcan->Instance == CAN1)
    {
        CAN_Receive_Callback(hcan, &msg);
    }
}
