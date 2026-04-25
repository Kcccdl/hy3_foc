#ifndef __PID_H
#define __PID_H

#include "stm32f4xx_hal.h"

// PID控制器结构体
typedef struct
{
    float Kp;          // 比例系数
    float Ki;          // 积分系数
    float Kd;          // 微分系数
    
    float integral;    // 积分项累计
    float prev_error;  // 上一次误差
    float max_integral;// 积分限幅
    float max_output;  // 输出限幅
    
    float target;      // 目标值
    float actual;      // 实际值
    float output;      // PID输出
} PID_t;

// PID控制器初始化
void PID_Init(PID_t *pid, float Kp, float Ki, float Kd, float max_integral, float max_output);

// PID参数设置
void PID_SetParams(PID_t *pid, float Kp, float Ki, float Kd);

// PID目标值设置
void PID_SetTarget(PID_t *pid, float target);

// PID计算函数 (位置式PID)
float PID_Calculate(PID_t *pid, float actual, float dt);

// PID计算函数 (增量式PID)
float PID_Calculate_Incremental(PID_t *pid, float actual, float dt);

// PID重置
void PID_Reset(PID_t *pid);

// PID积分清零
void PID_ClearIntegral(PID_t *pid);

#endif /* __PID_H */
