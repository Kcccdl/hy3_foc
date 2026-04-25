#include "pid.h"

// PID控制器初始化
void PID_Init(PID_t *pid, float Kp, float Ki, float Kd, float max_integral, float max_output)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->max_integral = max_integral;
    pid->max_output = max_output;
    
    pid->target = 0.0f;
    pid->actual = 0.0f;
    pid->output = 0.0f;
}

// PID参数设置
void PID_SetParams(PID_t *pid, float Kp, float Ki, float Kd)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
}

// PID目标值设置
void PID_SetTarget(PID_t *pid, float target)
{
    pid->target = target;
}

// PID计算函数 (位置式PID)
float PID_Calculate(PID_t *pid, float actual, float dt)
{
    float error;
    float derivative;
    
    pid->actual = actual;
    
    error = pid->target - actual;
    
    // 比例项
    float proportional = pid->Kp * error;
    
    // 积分项 (带限幅)
    pid->integral += pid->Ki * error * dt;
    
    if (pid->integral > pid->max_integral)
        pid->integral = pid->max_integral;
    else if (pid->integral < -pid->max_integral)
        pid->integral = -pid->max_integral;
    
    // 微分项
    derivative = pid->Kd * (error - pid->prev_error) / dt;
    pid->prev_error = error;
    
    pid->output = proportional + pid->integral + derivative;
    
    if (pid->output > pid->max_output)
        pid->output = pid->max_output;
    else if (pid->output < -pid->max_output)
        pid->output = -pid->max_output;
    
    return pid->output;
}

// PID计算函数 (增量式PID)
float PID_Calculate_Incremental(PID_t *pid, float actual, float dt)
{
    float error;
    float delta;
    
    pid->actual = actual;
    
    error = pid->target - actual;
    
    delta = pid->Kp * (error - pid->prev_error);
    delta += pid->Ki * error * dt;
    
    pid->prev_error = error;
    
    if (delta > pid->max_output)
        delta = pid->max_output;
    else if (delta < -pid->max_output)
        delta = -pid->max_output;
    
    pid->output += delta;
    
    return pid->output;
}

// PID重置
void PID_Reset(PID_t *pid)
{
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->output = 0.0f;
}

// PID积分清零
void PID_ClearIntegral(PID_t *pid)
{
    pid->integral = 0.0f;
}
