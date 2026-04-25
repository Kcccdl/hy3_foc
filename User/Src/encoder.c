#include "encoder.h"
#include "spi.h"
#include "math.h"

#define DEG_TO_RAD (3.141592653589793f / 180.0f)
#define RAD_TO_DEG (180.0f / 3.141592653589793f)

extern SPI_HandleTypeDef hspi1;

// 编码器初始化
void Encoder_Init(Encoder_t *encoder)
{
    encoder->angle = 0.0f;
    encoder->prev_angle = 0.0f;
    encoder->speed = 0.0f;
    encoder->angle_raw = 0.0f;
    encoder->encoder_value = 0;
    encoder->encoder_prev = 0;
    encoder->angle_offset = 0.0f;
    encoder->timestamp = 0;
    encoder->prev_timestamp = 0;
}

// 读取KTM5910磁编码器角度
void KTM5910_ReadAngle(Encoder_t *encoder)
{
    uint8_t tx_buf[2];
    uint8_t rx_buf[2];
    uint16_t raw_angle;
    
    tx_buf[0] = 0x00;
    tx_buf[1] = 0x00;
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    
    raw_angle = ((uint16_t)rx_buf[0] << 8) | rx_buf[1];
    raw_angle = raw_angle & 0x3FFF;
    
    encoder->encoder_value = raw_angle;
    encoder->angle_raw = (float)raw_angle * 360.0f / 16384.0f;
    encoder->angle = encoder->angle_raw * DEG_TO_RAD + encoder->angle_offset;
    
    while(encoder->angle >= 2 * 3.141592653589793f)
        encoder->angle -= 2 * 3.141592653589793f;
    while(encoder->angle < 0)
        encoder->angle += 2 * 3.141592653589793f;
}

float Encoder_GetAngle(Encoder_t *encoder)
{
    return encoder->angle;
}

float Encoder_GetSpeed(Encoder_t *encoder, float dt)
{
    float angle_diff;
    
    angle_diff = encoder->angle - encoder->prev_angle;
    
    if(angle_diff > 3.141592653589793f)
        angle_diff -= 2 * 3.141592653589793f;
    else if(angle_diff < -3.141592653589793f)
        angle_diff += 2 * 3.141592653589793f;
    
    if(dt > 0)
        encoder->speed = angle_diff / dt;
    else
        encoder->speed = 0.0f;
    
    return encoder->speed;
}

void Encoder_Update(Encoder_t *encoder, float dt)
{
    encoder->prev_angle = encoder->angle;
    KTM5910_ReadAngle(encoder);
    Encoder_GetSpeed(encoder, dt);
}

void Encoder_SetOffset(Encoder_t *encoder, float offset)
{
    encoder->angle_offset = offset;
}

void Encoder_Calibrate(Encoder_t *encoder)
{
    KTM5910_ReadAngle(encoder);
    encoder->angle_offset = -encoder->angle;
    KTM5910_ReadAngle(encoder);
}

uint16_t SPI_Read_16Bits(uint8_t reg)
{
    uint8_t tx_buf[3];
    uint8_t rx_buf[3];
    uint16_t data;
    
    tx_buf[0] = reg | 0x80;
    tx_buf[1] = 0x00;
    tx_buf[2] = 0x00;
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, 3, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    
    data = ((uint16_t)rx_buf[1] << 8) | rx_buf[2];
    return data;
}

void SPI_Write_16Bits(uint8_t reg, uint16_t data)
{
    uint8_t tx_buf[3];
    
    tx_buf[0] = reg & 0x7F;
    tx_buf[1] = (data >> 8) & 0xFF;
    tx_buf[2] = data & 0xFF;
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, tx_buf, 3, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}
