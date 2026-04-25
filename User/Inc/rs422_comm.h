#ifndef __RS422_COMM_H_
#define __RS422_COMM_H_

#include "stm32f4xx_hal.h"
#include "usart.h"

// RS422通信相关定义 (通过UART转RS422)
#define RS422_UART_HANDLE huart2  // 根据实际配置修改

// 通信协议定义
#define RS422_PROTOCOL_MODBUS 0   // Modbus RTU协议
#define RS422_PROTOCOL_CUSTOM 1   // 自定义协议

// 帧格式定义 (自定义协议)
// [0xAA][0x55][CMD][LEN][DATA...][CRC_LOW][CRC_HIGH]
#define RS422_FRAME_HEADER1  0xAA
#define RS422_FRAME_HEADER2  0x55

// 命令定义
#define RS422_CMD_SET_SPEED    0x01   // 设置速度
#define RS422_CMD_GET_SPEED    0x02   // 读取速度
#define RS422_CMD_SET_CURRENT  0x03   // 设置电流
#define RS422_CMD_GET_CURRENT  0x04   // 读取电流
#define RS422_CMD_START        0x10   // 启动电机
#define RS422_CMD_STOP         0x11   // 停止电机
#define RS422_CMD_GET_STATUS   0x20   // 读取状态
#define RS422_CMD_SET_PARAM    0x30   // 设置参数
#define RS422_CMD_GET_PARAM    0x31   // 读取参数

// 接收状态机
typedef enum
{
    RX_STATE_IDLE = 0,       // 空闲
    RX_STATE_HEADER1 = 1,    // 收到帧头1
    RX_STATE_HEADER2 = 2,    // 收到帧头2
    RX_STATE_CMD = 3,        // 收到命令
    RX_STATE_LEN = 4,        // 收到长度
    RX_STATE_DATA = 5,       // 接收数据
    RX_STATE_CRC = 6         // 接收CRC
} RX_State_t;

// RS422通信结构体
typedef struct
{
    uint8_t rx_buffer[64];   // 接收缓冲区
    uint8_t tx_buffer[64];   // 发送缓冲区
    uint8_t rx_index;        // 接收索引
    uint8_t rx_len;          // 接收数据长度
    uint8_t rx_cmd;          // 接收的命令
    RX_State_t rx_state;     // 接收状态
    uint8_t frame_ready;     // 帧接收完成标志
    uint32_t rx_timeout;     // 接收超时
    uint32_t last_rx_time;   // 上次接收时间
} RS422_Comm_t;

// 函数声明
void RS422_Init(void);
void RS422_Send_Data(uint8_t *data, uint8_t len);
void RS422_Send_Response(uint8_t cmd, uint8_t *data, uint8_t len);
void RS422_Process_Byte(uint8_t byte);
void RS422_Process_Frame(void);
uint16_t RS422_CRC16(uint8_t *data, uint8_t len);
void RS422_Send_Speed(float speed_rpm);
void RS422_Send_Status(uint8_t status);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

extern RS422_Comm_t rs422_comm;

#endif /* __RS422_COMM_H_ */
