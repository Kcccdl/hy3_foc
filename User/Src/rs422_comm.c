#include "rs422_comm.h"
#include "bldc.h"
#include "string.h"

//UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart2;
RS422_Comm_t rs422_comm = {0};
extern BLDC_Motor_t motor;

void RS422_Init(void)
{
    rs422_comm.rx_index = 0;
    rs422_comm.rx_len = 0;
    rs422_comm.rx_cmd = 0;
    rs422_comm.rx_state = RX_STATE_IDLE;
    rs422_comm.frame_ready = 0;
    rs422_comm.rx_timeout = 0;
    rs422_comm.last_rx_time = 0;
    
    uint8_t rx_byte;
    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
}

void RS422_Send_Data(uint8_t *data, uint8_t len)
{
    HAL_UART_Transmit(&huart2, data, len, 100);
}

void RS422_Send_Response(uint8_t cmd, uint8_t *data, uint8_t len)
{
    uint8_t frame[64];
    uint16_t crc;
    uint8_t index = 0;
    
    frame[index++] = RS422_FRAME_HEADER1;
    frame[index++] = RS422_FRAME_HEADER2;
    frame[index++] = cmd;
    frame[index++] = len;
    
    if(data != NULL && len > 0)
    {
        memcpy(&frame[index], data, len);
        index += len;
    }
    
    crc = RS422_CRC16(frame, index);
    frame[index++] = crc & 0xFF;
    frame[index++] = (crc >> 8) & 0xFF;
    
    HAL_UART_Transmit(&huart2, frame, index, 100);
}

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
                rs422_comm.rx_state = RX_STATE_HEADER2;
            else
                rs422_comm.rx_state = RX_STATE_IDLE;
            break;
            
        case RX_STATE_HEADER2:
            rs422_comm.rx_cmd = byte;
            rs422_comm.rx_state = RX_STATE_CMD;
            break;
            
        case RX_STATE_CMD:
            rs422_comm.rx_len = byte;
            if(rs422_comm.rx_len > 0 && rs422_comm.rx_len <= 56)
            {
                rs422_comm.rx_state = RX_STATE_DATA;
                rs422_comm.rx_index = 0;
            }
            else if(rs422_comm.rx_len == 0)
                rs422_comm.rx_state = RX_STATE_CRC;
            else
                rs422_comm.rx_state = RX_STATE_IDLE;
            break;
            
        case RX_STATE_DATA:
            rs422_comm.rx_buffer[rs422_comm.rx_index++] = byte;
            if(rs422_comm.rx_index >= rs422_comm.rx_len)
                rs422_comm.rx_state = RX_STATE_CRC;
            break;
            
        case RX_STATE_CRC:
            rs422_comm.frame_ready = 1;
            rs422_comm.rx_state = RX_STATE_IDLE;
            break;
            
        default:
            rs422_comm.rx_state = RX_STATE_IDLE;
            break;
    }
}

void RS422_Process_Frame(void)
{
    uint8_t response_data[8];
    float value;
    
    if(!rs422_comm.frame_ready)
        return;
    
    rs422_comm.frame_ready = 0;
    
    switch(rs422_comm.rx_cmd)
    {
        case RS422_CMD_SET_SPEED:
            if(rs422_comm.rx_len >= 4)
            {
                memcpy(&value, rs422_comm.rx_buffer, 4);
                BLDC_SetSpeed(&motor, value);
                RS422_Send_Response(RS422_CMD_SET_SPEED, (uint8_t*)"OK", 2);
            }
            break;
            
        case RS422_CMD_GET_SPEED:
            value = motor.encoder.speed * 60.0f / (2 * 3.141592653589793f);
            memcpy(response_data, &value, 4);
            RS422_Send_Response(RS422_CMD_GET_SPEED, response_data, 4);
            break;
            
        case RS422_CMD_SET_CURRENT:
            if(rs422_comm.rx_len >= 4)
            {
                memcpy(&value, rs422_comm.rx_buffer, 4);
                BLDC_SetCurrent(&motor, value);
                RS422_Send_Response(RS422_CMD_SET_CURRENT, (uint8_t*)"OK", 2);
            }
            break;
            
        case RS422_CMD_GET_CURRENT:
            value = motor.Ia;
            memcpy(response_data, &value, 4);
            RS422_Send_Response(RS422_CMD_GET_CURRENT, response_data, 4);
            break;
            
        case RS422_CMD_START:
            BLDC_Start(&motor);
            RS422_Send_Response(RS422_CMD_START, (uint8_t*)"OK", 2);
            break;
            
        case RS422_CMD_STOP:
            BLDC_Stop(&motor);
            RS422_Send_Response(RS422_CMD_STOP, (uint8_t*)"OK", 2);
            break;
            
        case RS422_CMD_GET_STATUS:
            response_data[0] = motor.state;
            response_data[1] = motor.mode;
            RS422_Send_Response(RS422_CMD_GET_STATUS, response_data, 2);
            break;
            
        default:
            RS422_Send_Response(0xFF, (uint8_t*)"ERR", 3);
            break;
    }
}

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
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    
    return crc;
}

void RS422_Send_Speed(float speed_rpm)
{
    uint8_t data[4];
    memcpy(data, &speed_rpm, 4);
    RS422_Send_Response(RS422_CMD_GET_SPEED, data, 4);
}

void RS422_Send_Status(uint8_t status)
{
    RS422_Send_Response(RS422_CMD_GET_STATUS, &status, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    static uint8_t rx_byte;
    
    if(huart->Instance == USART2)
    {
        RS422_Process_Byte(rx_byte);
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    }
}
