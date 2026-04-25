#include "current_sense.h"
#include "math.h"

PhaseCurrent_t phase_current = {0};

// 电流采样初始化
void CurrentSense_Init(PhaseCurrent_t *current)
{
    current->Ia = 0.0f;
    current->Ib = 0.0f;
    current->Ic = 0.0f;
    current->ready = 0;
    
    for(int i = 0; i < 3; i++)
        current->adc_raw[i] = 0;
}

// 启动电流采样 (使用ADC注入组或规则组+DMA)
void CurrentSense_Start(PhaseCurrent_t *current)
{
    // 使用规则组+DMA方式采样三相电流
    // 假设ADC1采样Ia, ADC2采样Ib, ADC3采样Ic
    // 或者使用同一个ADC的多通道扫描+DMA
    /*
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)current->adc_raw, 3);
    */
}

// 停止电流采样
void CurrentSense_Stop(PhaseCurrent_t *current)
{
    /*
    HAL_ADC_Stop_DMA(&hadc1);
    */
    current->ready = 0;
}

// 读取ADC值并转换为电流
void CurrentSense_ReadADC(PhaseCurrent_t *current)
{
    uint32_t sum[3] = {0};
    uint16_t adc_value[3];
    
    // 多次采样取平均
    for(int i = 0; i < CURRENT_SAMPLE_COUNT; i++)
    {
        // 触发ADC采样
        // HAL_ADC_Start(&hadc1);
        // HAL_ADC_PollForConversion(&hadc1, 10);
        
        // 读取ADC值 (根据实际硬件修改)
        // adc_value[0] = HAL_ADC_GetValue(&hadc1);  // Ia
        // HAL_ADC_Start(&hadc2);
        // adc_value[1] = HAL_ADC_GetValue(&hadc2);  // Ib
        // HAL_ADC_Start(&hadc3);
        // adc_value[2] = HAL_ADC_GetValue(&hadc3);  // Ic
        
        // 这里使用模拟数据作为示例
        adc_value[0] = 2048;
        adc_value[1] = 2048;
        adc_value[2] = 2048;
        
        sum[0] += adc_value[0];
        sum[1] += adc_value[1];
        sum[2] += adc_value[2];
    }
    
    // 计算平均值
    current->adc_raw[0] = sum[0] / CURRENT_SAMPLE_COUNT;
    current->adc_raw[1] = sum[1] / CURRENT_SAMPLE_COUNT;
    current->adc_raw[2] = sum[2] / CURRENT_SAMPLE_COUNT;
    
    // 转换为实际电流值
    current->Ia = CurrentSense_ConvertToCurrent(current->adc_raw[0]);
    current->Ib = CurrentSense_ConvertToCurrent(current->adc_raw[1]);
    current->Ic = CurrentSense_ConvertToCurrent(current->adc_raw[2]);
    
    // 根据Ia + Ib + Ic = 0计算第三相 (如果只采样两相)
    // current->Ic = -(current->Ia + current->Ib);
    
    current->ready = 1;
}

// ADC值转换为电流 (安培)
// 公式: I = (ADC - Offset) * Vref / (Resolution * Rshunt * Gain)
float CurrentSense_ConvertToCurrent(uint16_t adc_value)
{
    float voltage;
    float current;
    
    // ADC值转换为电压
    voltage = ((float)adc_value - CURRENT_OFFSET) * ADC_REF_VOLTAGE / ADC_RESOLUTION;
    
    // 电压转换为电流 (通过采样电阻和运放)
    current = voltage / (SHUNT_RESISTOR * AMP_GAIN);
    
    return current;
}

// 校准零电流偏移
void CurrentSense_CalibrateOffset(PhaseCurrent_t *current)
{
    uint32_t sum[3] = {0};
    uint16_t adc_value;
    
    // 电机不运行时采样
    for(int i = 0; i < 100; i++)
    {
        // 采样ADC
        // adc_value = HAL_ADC_GetValue(&hadc1);
        // sum[0] += adc_value;
        
        // 示例
        sum[0] += 2048;
        sum[1] += 2048;
        sum[2] += 2048;
        
        HAL_Delay(1);
    }
    
    // 计算新的偏移值
    // CURRENT_OFFSET = sum[0] / 100;
}

// ADC转换完成回调 (如果使用中断方式)
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    // 读取转换结果
    // phase_current.adc_raw[0] = HAL_ADC_GetValue(hadc);
    // phase_current.ready = 1;
}
