#include "pid.h"

// PID控制器初始化
void PID_Init(PID_t *pid, float Kp, float Ki, float Kd, float max_integral, float max_output)
{
    pid->Kp = Kp;                // 设置比例系数
    pid->Ki = Ki;                // 设置积分系数
    pid->Kd = Kd;                // 设置微分系数
    
    pid->integral = 0.0f;        // 积分项清零
    pid->prev_error = 0.0f;      // 上一次误差清零
    pid->max_integral = max_integral;  // 积分限幅值
    pid->max_output = max_output;      // 输出限幅值
    
    pid->target = 0.0f;          // 目标值清零
    pid->actual = 0.0f;          // 实际值清零
    pid->output = 0.0f;          // 输出值清零
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
// actual: 实际值
// dt: 采样时间间隔 (秒)
float PID_Calculate(PID_t *pid, float actual, float dt)
{
    float error;      // 当前误差
    float derivative; // 微分项
    
    // 更新实际值
    pid->actual = actual;
    
    // 计算误差
    error = pid->target - actual;
    
    // 比例项
    float proportional = pid->Kp * error;
    
    // 积分项 (带限幅)
    pid->integral += pid->Ki * error * dt;
    
    // 积分限幅
    if (pid->integral > pid->max_integral)
        pid->integral = pid->max_integral;
    else if (pid->integral < -pid->max_integral)
        pid->integral = -pid->max_integral;
    
    // 微分项
    derivative = pid->Kd * (error - pid->prev_error) / dt;
    pid->prev_error = error;
    
    // 计算PID输出
    pid->output = proportional + pid->integral + derivative;
    
    // 输出限幅
    if (pid->output > pid->max_output)
        pid->output = pid->max_output;
    else if (pid->output < -pid->max_output)
        pid->output = -pid->max_output;
    
    return pid->output;
}

// PID计算函数 (增量式PID)
float PID_Calculate_Incremental(PID_t *pid, float actual, float dt)
{
    float error;      // 当前误差
    float delta;      // 增量输出
    
    // 更新实际值
    pid->actual = actual;
    
    // 计算误差
    error = pid->target - actual;
    
    // 增量式PID计算公式
    // delta = Kp*(error - prev_error) + Ki*error*dt + Kd*(error - 2*prev_error + prev_prev_error)/dt
    // 这里简化为: delta = Kp*error - Kp*prev_error + Ki*error*dt...
    
    delta = pid->Kp * (error - pid->prev_error);
    delta += pid->Ki * error * dt;
    // 注意：增量式需要保存上上次误差，这里简化处理
    
    pid->prev_error = error;
    
    // 输出限幅
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
