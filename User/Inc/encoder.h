#ifndef __ENCODER_H
#define __ENCODER_H

#include "stm32f4xx_hal.h"

// KTM5910磁编码器相关定义
#define KTM5910_SPI_HANDLE hspi1  // 根据实际配置修改

// 编码器结构体
typedef struct
{
    float angle;             // 当前机械角度 (弧度)
    float prev_angle;        // 上一次机械角度
    float speed;             // 转速 (rad/s)
    float angle_raw;         // 原始角度值 (0-360度或0-2PI)
    uint16_t encoder_value;  // 编码器原始数值
    uint16_t encoder_prev;   // 上一次编码器数值
    float angle_offset;      // 角度偏移校准值
    uint32_t timestamp;      // 时间戳
    uint32_t prev_timestamp; // 上一次时间戳
} Encoder_t;

// 函数声明
void Encoder_Init(Encoder_t *encoder);
void KTM5910_ReadAngle(Encoder_t *encoder);
float Encoder_GetAngle(Encoder_t *encoder);
float Encoder_GetSpeed(Encoder_t *encoder, float dt);
void Encoder_Update(Encoder_t *encoder, float dt);
void Encoder_SetOffset(Encoder_t *encoder, float offset);
void Encoder_Calibrate(Encoder_t *encoder);

// SPI读取函数 (需要根据实际硬件实现)
uint16_t SPI_Read_16Bits(uint8_t reg);
void SPI_Write_16Bits(uint8_t reg, uint16_t data);

#endif /* __ENCODER_H */
