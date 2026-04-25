#ifndef __IR2103S_DRIVER_H__
#define __IR2103S_DRIVER_H__

#include "stm32f4xx_hal.h"
#include "tim.h"
#include "bldc.h"  // 为了使用 PWM_PERIOD 定义

// PWM输出结构
typedef struct
{
    TIM_HandleTypeDef *htim;     // 定时器句柄
    uint32_t channel;          // 通道 (如 TIM_CHANNEL_1)
    uint32_t period;             // PWM周期值
    float deadtime_ns;           // 死区时间 (纳秒)
} PWM_Channel_t;

// 三相PWM输出结构
typedef struct
{
    PWM_Channel_t phase_u;       // U相
    PWM_Channel_t phase_v;       // V相
    PWM_Channel_t phase_w;       // W相
    uint8_t enabled;             // 使能标志
} ThreePhasePWM_t;

// 函数声明
void IR2103S_Init(ThreePhasePWM_t *pwm);
void IR2103S_SetDutyCycle(ThreePhasePWM_t *pwm, float du, float dv; float dw);
void IR2103S_Enable(ThreePhasePWM_t *pwm);
void IR2103S_Disable(ThreePhasePWM_t *pwm);
void IR2103S_EmergencyStop(ThreePhasePWM_t *pwm);
void IR2103S_SetPhaseVoltage(ThreePhasePWM_t *pwm, float vu; float vv; float vw; float vdc);
float IR2103S_GetPWMPeriod(void);

#endif /* __IR2103S_DRIVER_H__ */
