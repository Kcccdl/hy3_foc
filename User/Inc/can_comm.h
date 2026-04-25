#ifndef __CAN_COMM_H__
#define __CAN_COMM_H__

#include "stm32f4xx_hal.h"
#include "can.h"
#include "bldc.h"

// CAN通信相关定义
#define CAN_HTIMER_HANDLE htim2  // 用于CAN超时检测，根据实际配置修改

// CAN消息ID定义
#define CAN_ID_COMMAND_SPEED    0x100   // 速度指令
#define CAN_ID_COMMAND_CURRENT  0x101   // 电流指令
#define CAN_ID_STATUS           0x200   // 状态反馈
#define CAN_ID_HEARTBEAT        0x201   // 心跳包
#define CAN_ID_PARAM_SET        0x300   // 参数设置
#define CAN_ID_PARAM_GET        0x301   // 参数读取

// CAN消息结构体
typedef struct
{
    uint32_t id;           // CAN ID
    uint8_t data[8];       // 数据 (8字节)
    uint8_t len;           // 数据长度
    uint8_t format;        // 格式: 0=标准帧, 1=扩展帧
} CAN_Message_t;

// CAN通信状态
typedef struct
{
    uint8_t connected;      // 连接状态
    uint32_t tx_count;      // 发送计数
    uint32_t rx_count;      // 接收计数
    uint32_t error_count;   // 错误计数
    uint32_t last_rx_time;  // 上次接收时间
} CAN_Status_t;

// 函数声明
void CAN_Comm_Init(void);
void CAN_Send_Message(uint32_t id, uint8_t *data, uint8_t len);
void CAN_Receive_Callback(CAN_HandleTypeDef *hcan, CAN_Message_t *msg);
void CAN_Process_Command(CAN_Message_t *msg);
void CAN_Send_Status(float speed, float current);
void CAN_Send_Heartbeat(void);
void CAN_Set_Baudrate(uint32_t baudrate);
void CAN_Filter_Config(void);
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);

extern CAN_Status_t can_status;
extern BLDC_Motor_t motor;  // 声明外部变量

#endif /* __CAN_COMM_H__ */
