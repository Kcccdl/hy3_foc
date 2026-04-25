#include "ir2103s_driver.h"

// IR2103S驱动初始化
void IR2103S_Init(ThreePhasePWM_t *pwm)
{
    // 初始化U相
    pwm->phase_u.htim = &htim1;          // 使用TIM1
    pwm->phase_u.channel_high = TIM_CHANNEL_1;
    pwm->phase_u.channel_low = TIM_CHANNEL_1N;  // 互补通道
    pwm->phase_u.period = PWM_PERIOD;    // 与CubeMX配置一致
    pwm->phase_u.deadtime_ns = 200.0f;  // 200ns死区
    
    // 初始化V相
    pwm->phase_v.htim = &htim1;
    pwm->phase_v.channel_high = TIM_CHANNEL_2;
    pwm->phase_v.channel_low = TIM_CHANNEL_2N;
    pwm->phase_v.period = PWM_PERIOD;
    pwm->phase_v.deadtime_ns = 200.0f;
    
    // 初始化W相
    pwm->phase_w.htim = &htim1;
    pwm->phase_w.channel_high = TIM_CHANNEL_3;
    pwm->phase_w.channel_low = TIM_CHANNEL_3N;
    pwm->phase_w.period = PWM_PERIOD;
    pwm->phase_w.deadtime_ns = 200.0f;
    
    // 初始状态为使能关闭
    pwm->enabled = 0;
    
    // 停止PWM输出 (占空比设为0)
    IR2103S_SetDutyCycle(pwm, 0.0f, 0.0f, 0.0f);
}

// 设置三相PWM占空比 (0.0 - 1.0)
void IR2103S_SetDutyCycle(ThreePhasePWM_t *pwm, float du, float dv, float dw)
{
    uint32_t compare_u, compare_v, compare_w;
    
    // 限制占空比范围
    if(du > 1.0f) du = 1.0f;
    if(du < 0.0f) du = 0.0f;
    if(dv > 1.0f) dv = 1.0f;
    if(dv < 0.0f) dv = 0.0f;
    if(dw > 1.0f) dw = 1.0f;
    if(dw < 0.0f) dw = 0.0f;
    
    // 计算比较值
    compare_u = (uint32_t)(du * pwm->phase_u.period);
    compare_v = (uint32_t)(dv * pwm->phase_v.period);
    compare_w = (uint32_t)(dw * pwm->phase_w.period);
    
    // 设置PWM比较值
    __HAL_TIM_SET_COMPARE(pwm->phase_u.htim, pwm->phase_u.channel_high, compare_u);
    __HAL_TIM_SET_COMPARE(pwm->phase_v.htim, pwm->phase_v.channel_high, compare_v);
    __HAL_TIM_SET_COMPARE(pwm->phase_w.htim, pwm->phase_w.channel_high, compare_w);
}

// 使能PWM输出
void IR2103S_Enable(ThreePhasePWM_t *pwm)
{
    // 启动PWM输出 (高侧和低侧)
    HAL_TIM_PWM_Start(pwm->phase_u.htim, pwm->phase_u.channel_high);
    HAL_TIMEx_PWMN_Start(pwm->phase_u.htim, pwm->phase_u.channel_low);
    
    HAL_TIM_PWM_Start(pwm->phase_v.htim, pwm->phase_v.channel_high);
    HAL_TIMEx_PWMN_Start(pwm->phase_v.htim, pwm->phase_v.channel_low);
    
    HAL_TIM_PWM_Start(pwm->phase_w.htim, pwm->phase_w.channel_high);
    HAL_TIMEx_PWMN_Start(pwm->phase_w.htim, pwm->phase_w.channel_low);
    
    pwm->enabled = 1;
}

// 禁用PWM输出
void IR2103S_Disable(ThreePhasePWM_t *pwm)
{
    // 停止PWM输出
    HAL_TIM_PWM_Stop(pwm->phase_u.htim, pwm->phase_u.channel_high);
    HAL_TIMEx_PWMN_Stop(pwm->phase_u.htim, pwm->phase_u.channel_low);
    
    HAL_TIM_PWM_Stop(pwm->phase_v.htim, pwm->phase_v.channel_high);
    HAL_TIMEx_PWMN_Stop(pwm->phase_v.htim, pwm->phase_v.channel_low);
    
    HAL_TIM_PWM_Stop(pwm->phase_w.htim, pwm->phase_w.channel_high);
    HAL_TIMEx_PWMN_Stop(pwm->phase_w.htim, pwm->phase_w.channel_low);
    
    pwm->enabled = 0;
}

// 紧急停止 (所有输出为0)
void IR2103S_EmergencyStop(ThreePhasePWM_t *pwm)
{
    // 设置占空比为0
    IR2103S_SetDutyCycle(pwm, 0.0f, 0.0f, 0.0f);
    
    // 可以触发刹车功能 (如果配置了TIM_Break)
    // HAL_TIM_GenerateEvent(pwm->phase_u.htim, TIM_EVENTSOURCE_BREAK);
}

// 根据电压值设置PWM (用于SVPWM)
// vu, vv, vw: 相电压 (-vdc/2 到 +vdc/2)
// vdc: 直流母线电压
void IR2103S_SetPhaseVoltage(ThreePhasePWM_t *pwm, float vu, float vv, float vw, float vdc)
{
    float du, dv, dw;
    
    // 将电压转换为占空比 (加入中点偏移，使电压全为正)
    // 实际使用中，SVPWM函数会直接输出占空比
    du = (vu / vdc) + 0.5f;
    dv = (vv / vdc) + 0.5f;
    dw = (vw / vdc) + 0.5f;
    
    // 限制范围
    if(du > 1.0f) du = 1.0f;
    if(du < 0.0f) du = 0.0f;
    if(dv > 1.0f) dv = 1.0f;
    if(dv < 0.0f) dv = 0.0f;
    if(dw > 1.0f) dw = 1.0f;
    if(dw < 0.0f) dw = 0.0f;
    
    // 设置PWM
    IR2103S_SetDutyCycle(pwm, du, dv, dw);
}

// 获取PWM周期值
float IR2103S_GetPWMPeriod(void)
{
    return (float)PWM_PERIOD;
}
