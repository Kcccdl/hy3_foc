#include "current_sense.h"
#include "math.h"

PhaseCurrent_t phase_current = {0};

void CurrentSense_Init(PhaseCurrent_t *current)
{
    current->Ia = 0.0f;
    current->Ib = 0.0f;
    current->Ic = 0.0f;
    current->ready = 0;
    
    for(int i = 0; i < 3; i++)
        current->adc_raw[i] = 0;
}

void CurrentSense_Start(PhaseCurrent_t *current)
{
    /*
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)current->adc_raw, 3);
    */
}

void CurrentSense_Stop(PhaseCurrent_t *current)
{
    /*
    HAL_ADC_Stop_DMA(&hadc1);
    */
    current->ready = 0;
}

void CurrentSense_ReadADC(PhaseCurrent_t *current)
{
    uint32_t sum[3] = {0};
    uint16_t adc_value[3];
    
    for(int i = 0; i < CURRENT_SAMPLE_COUNT; i++)
    {
        adc_value[0] = 2048;
        adc_value[1] = 2048;
        adc_value[2] = 2048;
        
        sum[0] += adc_value[0];
        sum[1] += adc_value[1];
        sum[2] += adc_value[2];
    }
    
    current->adc_raw[0] = sum[0] / CURRENT_SAMPLE_COUNT;
    current->adc_raw[1] = sum[1] / CURRENT_SAMPLE_COUNT;
    current->adc_raw[2] = sum[2] / CURRENT_SAMPLE_COUNT;
    
    current->Ia = CurrentSense_ConvertToCurrent(current->adc_raw[0]);
    current->Ib = CurrentSense_ConvertToCurrent(current->adc_raw[1]);
    current->Ic = CurrentSense_ConvertToCurrent(current->adc_raw[2]);
    
    current->ready = 1;
}

float CurrentSense_ConvertToCurrent(uint16_t adc_value)
{
    float voltage;
    float current;
    
    voltage = ((float)adc_value - CURRENT_OFFSET) * ADC_REF_VOLTAGE / ADC_RESOLUTION;
    current = voltage / (SHUNT_RESISTOR * AMP_GAIN);
    
    return current;
}

void CurrentSense_CalibrateOffset(PhaseCurrent_t *current)
{
    uint32_t sum[3] = {0};
    
    for(int i = 0; i < 100; i++)
    {
        sum[0] += 2048;
        sum[1] += 2048;
        sum[2] += 2048;
        HAL_Delay(1);
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    // phase_current.adc_raw[0] = HAL_ADC_GetValue(hadc);
    // phase_current.ready = 1;
}
