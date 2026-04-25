#include "rs422_comm.h"
#include "bldc.h"
#include "string.h"

UART_HandleTypeDef huart2;  // CubeMX生成的UART句柄
RS422_Comm_t rs422_comm = {0};
extern BLDC_Motor_t motor;

// RS422初始化
void RS422_Init(void)
{
    // 初始化接收状态机
    rs422_comm.rx_index = 0;
    rs422_comm.rx_len = 0;
    rs422_comm.rx_cmd = 0;
    rs422_comm.rx_state = RX_STATE_IDLE;
    rs422_comm.frame_ready = 0;
    rs422_comm.rx_timeout = 0;
    rs422_comm.last_rx_time = 0;
    
    // 启动UART接收中断 (接收1个字节)
    uint8_t rx_byte;
    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
}

// 发送数据
void RS422_Send_Data(uint8_t *data, uint8_t len)
{
    // 发送数据 (阻塞方式)
    HAL_UART_Transmit(&huart2, data, len, 100);
}

// 发送响应帧
void RS422_Send_Response(uint8_t cmd, uint8_t *data, uint8_t len)
{
    uint8_t frame[64];
    uint16_t crc;
    uint8_t index = 0;
    
    // 帧头
    frame[index++] = RS422_FRAME_HEADER1;
    frame[index++] = RS422_FRAME_HEADER2;
    
    // 命令
    frame[index++] = cmd;
    
    // 数据长度
    frame[index++] = len;
    
    // 数据
    if(data != NULL && len > 0)
    {
        memcpy(&frame[index], data, len);
        index += len;
    }
    
    // CRC16校验
    crc = RS422_CRC16(frame, index);
    frame[index++] = crc & 0xFF;        // CRC低字节
    frame[index++] = (crc >> 8) & 0xFF; // CRC高字节
    
    // 发送帧
    RS422_Send_Data(frame, index);
}

// 处理接收到的字节 (状态机方式)
void RS422_Process_Byte(uint8_t byte)
{
    switch(rs422_comm.rx_state)
    {
        case RX_STATE_IDLE:
            if(byte == RS422_FRAME_HEADER1)
            {
                rs422_comm.rx_state = RX_STATE_HEADER1;
                rs422_comm.rx_index = 0;
            }
            break;
            
        case RX_STATE_HEADER1:
            if(byte == RS422_FRAME_HEADER2)
            {
                rs422_comm.rx_state = RX_STATE_HEADER2;
            }
            else
            {
                rs422_comm.rx_state = RX_STATE_IDLE;  // 错误，复位
            }
            break;
            
        case RX_STATE_HEADER2:
            // 收到命令字节
            rs422_comm.rx_cmd = byte;
            rs422_comm.rx_state = RX_STATE_CMD;
            break;
            
        case RX_STATE_CMD:
            // 收到数据长度
            rs422_comm.rx_len = byte;
            if(rs422_comm.rx_len > 0 && rs422_comm.rx_len <= 56)  // 最大数据长度
            {
                rs422_comm.rx_state = RX_STATE_DATA;
                rs422_comm.rx_index = 0;
            }
            else if(rs422_comm.rx_len == 0)
            {
                rs422_comm.rx_state = RX_STATE_CRC;  // 无数据，直接到CRC
            }
            else
            {
                rs422_comm.rx_state = RX_STATE_IDLE;  // 错误长度
            }
            break;
            
        case RX_STATE_DATA:
            // 接收数据
            rs422_comm.rx_buffer[rs422_comm.rx_index++] = byte;
            if(rs422_comm.rx_index >= rs422_comm.rx_len)
            {
                rs422_comm.rx_state = RX_STATE_CRC;
            }
            break;
            
        case RX_STATE_CRC:
            // 这里简化，实际需要接收2字节CRC并校验
            rs422_comm.frame_ready = 1;  // 帧接收完成
            rs422_comm.rx_state = RX_STATE_IDLE;
            break;
            
        default:
            rs422_comm.rx_state = RX_STATE_IDLE;
            break;
    }
}

// 处理完整帧
void RS422_Process_Frame(void)
{
    uint8_t response_data[8];
    float value;
    
    if(!rs422_comm.frame_ready)
        return;
    
    rs422_comm.frame_ready = 0;  // 清除标志
    
    // 根据命令处理
    switch(rs422_comm.rx_cmd)
    {
        case RS422_CMD_SET_SPEED:
            // 设置速度: 4字节浮点数
            if(rs422_comm.rx_len >= 4)
            {
                memcpy(&value, rs422_comm.rx_buffer, 4);
                BLDC_SetSpeed(&motor, value);
                RS422_Send_Response(RS422_CMD_SET_SPEED, (uint8_t*)"OK", 2);
            }
            break;
            
        case RS422_CMD_GET_SPEED:
            // 读取速度
            value = motor.encoder.speed * 60.0f / (2 * 3.141592653589793f);  // 转换为RPM
            memcpy(response_data, &value, 4);
            RS422_Send_Response(RS422_CMD_GET_SPEED, response_data, 4);
            break;
            
        case RS422_CMD_SET_CURRENT:
            // 设置电流
            if(rs422_comm.rx_len >= 4)
            {
                memcpy(&value, rs422_comm.rx_buffer, 4);
                BLDC_SetCurrent(&motor, value);
                RS422_Send_Response(RS422_CMD_SET_CURRENT, (uint8_t*)"OK", 2);
            }
            break;
            
        case RS422_CMD_GET_CURRENT:
            // 读取电流
            value = motor.Ia;
            memcpy(response_data, &value, 4);
            RS422_Send_Response(RS422_CMD_GET_CURRENT, response_data, 4);
            break;
            
        case RS422_CMD_START:
            // 启动电机
            BLDC_Start(&motor);
            RS422_Send_Response(RS422_CMD_START, (uint8_t*)"OK", 2);
            break;
            
        case RS422_CMD_STOP:
            // 停止电机
            BLDC_Stop(&motor);
            RS422_Send_Response(RS422_CMD_STOP, (uint8_t*)"OK", 2);
            break;
            
        case RS422_CMD_GET_STATUS:
            // 读取状态
            response_data[0] = motor.state;
            response_data[1] = motor.mode;
            RS422_Send_Response(RS422_CMD_GET_STATUS, response_data, 2);
            break;
            
        default:
            // 未知命令
            RS422_Send_Response(0xFF, (uint8_t*)"ERR", 3);
            break;
    }
}

// CRC16计算 (Modbus多项式)
uint16_t RS422_CRC16(uint8_t *data, uint8_t len)
{
    uint16_t crc = 0xFFFF;
    uint8_t i, j;
    
    for(i = 0; i < len; i++)
    {
        crc ^= data[i];
        for(j = 0; j < 8; j++)
        {
            if(crc & 0x0001)
            {
                crc >>= 1;
                crc ^= 0xA001;  // Modbus多项式
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    
    return crc;
}

// 发送速度数据
void RS422_Send_Speed(float speed_rpm)
{
    uint8_t data[4];
    memcpy(data, &speed_rpm, 4);
    RS422_Send_Response(RS422_CMD_GET_SPEED, data, 4);
}

// 发送状态
void RS422_Send_Status(uint8_t status)
{
    RS422_Send_Response(RS422_CMD_GET_STATUS, &status, 1);
}

// UART接收完成回调 (HAL库)
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    static uint8_t rx_byte;
    
    if(huart->Instance == USART2)
    {
        // 处理接收到的字节
        RS422_Process_Byte(rx_byte);
        
        // 继续接收下一个字节
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    }
}
