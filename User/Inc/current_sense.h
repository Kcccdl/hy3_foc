#ifndef __CURRENT_SENSE_H
#define __CURRENT_SENSE_H

#include "stm32f4xx_hal.h"
#include "adc.h"

// 电流采样相关定义
#define CURRENT_SAMPLE_COUNT 16    // 每次采样次数 (用于平均)
#define CURRENT_OFFSET 2048         // ADC零电流偏移 (12位ADC中点)
#define ADC_REF_VOLTAGE 3.3f       // ADC参考电压
#define ADC_RESOLUTION 4096.0f      // 12位ADC分辨率
#define SHUNT_RESISTOR 0.01f       // 采样电阻 (欧姆)
#define AMP_GAIN 20.0f             // 运放放大倍数

// 三相电流结构体
typedef struct
{
    float Ia;              // A相电流 (安培)
    float Ib;              // B相电流 (安培)
    float Ic;              // C相电流 (安培)
    uint16_t adc_raw[3];   // 原始ADC值
    uint8_t ready;         // 采样完成标志
} PhaseCurrent_t;

// 函数声明
void CurrentSense_Init(PhaseCurrent_t *current);
void CurrentSense_Start(PhaseCurrent_t *current);
void CurrentSense_Stop(PhaseCurrent_t *current);
void CurrentSense_ReadADC(PhaseCurrent_t *current);
float CurrentSense_ConvertToCurrent(uint16_t adc_value);
void CurrentSense_CalibrateOffset(PhaseCurrent_t *current);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);

extern PhaseCurrent_t phase_current;

#endif /* __CURRENT_SENSE_H */
