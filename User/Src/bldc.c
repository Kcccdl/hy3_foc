#include "bldc.h"
#include "tim.h"
#include "math.h"

// BLDC电机初始化
void BLDC_Init(BLDC_Motor_t *motor)
{
    // 电机基本参数
    motor->pole_pairs = POLE_PAIRS;
    motor->max_speed = MAX_SPEED;
    motor->pwm_period = PWM_PERIOD;
    
    // 初始化电流值
    motor->Ia = 0.0f;
    motor->Ib = 0.0f;
    motor->Ic = 0.0f;
    
    // 初始化霍尔传感器
    motor->hall_state = 0;
    motor->hall_prev = 0;
    motor->hall_timestamp = 0;
    motor->hall_speed = 0.0f;
    
    // PWM占空比
    motor->duty_cycle = 0.0f;
    
    // 控制模式默认FOC
    motor->mode = COMMUTATION_FOC;
    motor->state = MOTOR_STOP;
    
    // 初始化PID控制器
    // 速度PID: Kp, Ki, Kd, 积分限幅, 输出限幅
    PID_Init(&motor->speed_pid, 0.5f, 0.1f, 0.01f, 100.0f, 30.0f);
    
    // 电流PID (q轴): Kp, Ki, Kd, 积分限幅, 输出限幅
    PID_Init(&motor->current_pid, 2.0f, 0.5f, 0.0f, 50.0f, 24.0f);
    
    // 初始化FOC控制器
    FOC_Init(&motor->foc);
    
    // 初始化编码器
    Encoder_Init(&motor->encoder);
    
    // 目标值
    motor->target_speed = 0.0f;
    motor->target_current = 0.0f;
    
    // 初始状态为停止，PWM输出为0
    BLDC_Stop(motor);
}

// 设置目标速度 (RPM)
void BLDC_SetSpeed(BLDC_Motor_t *motor, float speed_rpm)
{
    // 限制速度范围
    if(speed_rpm > motor->max_speed)
        speed_rpm = motor->max_speed;
    else if(speed_rpm < -motor->max_speed)
        speed_rpm = -motor->max_speed;
    
    motor->target_speed = speed_rpm;
    PID_SetTarget(&motor->speed_pid, speed_rpm);
}

// 设置目标电流 (A)
void BLDC_SetCurrent(BLDC_Motor_t *motor, float current_a)
{
    motor->target_current = current_a;
    PID_SetTarget(&motor->current_pid, current_a);
}

// 启动电机
void BLDC_Start(BLDC_Motor_t *motor)
{
    motor->state = MOTOR_RUN;
    
    // 使能PWM输出 (解除刹车)
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);  // 互补通道
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
}

// 停止电机
void BLDC_Stop(BLDC_Motor_t *motor)
{
    motor->state = MOTOR_STOP;
    motor->duty_cycle = 0.0f;
    
    // 设置PWM占空比为0
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
    
    // 可选：刹车或进入高阻态
}

// 六步换相函数
void BLDC_SixStep_Commutation(BLDC_Motor_t *motor)
{
    uint32_t pwm_value;
    
    // 计算PWM值
    pwm_value = (uint32_t)(motor->duty_cycle * motor->pwm_period);
    
    // 根据霍尔状态进行换相
    switch(motor->hall_state)
    {
        case 0x05:  // 霍尔状态 101 (A+B-)
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm_value);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
            break;
            
        case 0x04:  // 霍尔状态 100 (A+C-)
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm_value);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
            break;
            
        case 0x06:  // 霍尔状态 110 (B+C-)
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, pwm_value);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
            break;
            
        case 0x02:  // 霍尔状态 010 (B+A-)
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, pwm_value);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
            break;
            
        case 0x03:  // 霍尔状态 011 (C+A-)
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, pwm_value);
            break;
            
        case 0x01:  // 霍尔状态 001 (C+B-)
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, pwm_value);
            break;
            
        default:
            // 异常状态，停止PWM
            BLDC_Stop(motor);
            break;
    }
}

// FOC控制函数
void BLDC_FOC_Control(BLDC_Motor_t *motor, float dt)
{
    // 读取编码器角度
    Encoder_Update(&motor->encoder, dt);
    
    // 速度环PID (如果使用速度控制)
    if(motor->target_speed != 0)
    {
        float speed_rad_s = motor->encoder.speed;  // rad/s
        float speed_rpm = speed_rad_s * 60.0f / (2 * 3.141592653589793f);  // 转换为RPM
        
        // 速度PID计算，输出作为q轴电流参考
        float speed_output = PID_Calculate(&motor->speed_pid, speed_rpm, dt);
        motor->foc.idq_ref.q = speed_output;  // q轴电流参考
    }
    
    // 更新FOC
    FOC_Update(&motor->foc, motor->Ia, motor->Ib, motor->Ic, 
               motor->encoder.angle, dt);
    
    // 设置PWM输出到定时器
    uint32_t pwm_u = (uint32_t)(motor->foc.pwm.Ta * motor->pwm_period);
    uint32_t pwm_v = (uint32_t)(motor->foc.pwm.Tb * motor->pwm_period);
    uint32_t pwm_w = (uint32_t)(motor->foc.pwm.Tc * motor->pwm_period);
    
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm_u);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, pwm_v);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, pwm_w);
}

// 电机主更新函数
void BLDC_Update(BLDC_Motor_t *motor, float dt)
{
    if(motor->state != MOTOR_RUN)
        return;
    
    // 读取霍尔传感器
    BLDC_ReadHallSensors(motor);
    
    // 根据控制模式执行相应控制
    if(motor->mode == COMMUTATION_6STEP)
    {
        // 六步换相控制
        BLDC_SixStep_Commutation(motor);
    }
    else if(motor->mode == COMMUTATION_FOC)
    {
        // FOC控制
        BLDC_FOC_Control(motor, dt);
    }
}

// 设置三相PWM占空比 (0.0 - 1.0)
void BLDC_SetPWM(float duty_u, float duty_v, float duty_w)
{
    uint32_t pwm_period = PWM_PERIOD;
    
    // 限制占空比范围
    if(duty_u > 1.0f) duty_u = 1.0f;
    if(duty_u < 0.0f) duty_u = 0.0f;
    if(duty_v > 1.0f) duty_v = 1.0f;
    if(duty_v < 0.0f) duty_v = 0.0f;
    if(duty_w > 1.0f) duty_w = 1.0f;
    if(duty_w < 0.0f) duty_w = 0.0f;
    
    // 设置PWM比较值
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, (uint32_t)(duty_u * pwm_period));
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, (uint32_t)(duty_v * pwm_period));
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, (uint32_t)(duty_w * pwm_period));
}

// 读取霍尔传感器状态
void BLDC_ReadHallSensors(BLDC_Motor_t *motor)
{
    uint8_t hall_a, hall_b, hall_c;
    
    // 读取三个霍尔传感器引脚电平
    hall_a = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);  // 假设霍尔A接PA0
    hall_b = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1);  // 假设霍尔B接PA1
    hall_c = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2);  // 假设霍尔C接PA2
    
    // 组合成3位霍尔状态 (顺序: C-B-A)
    motor->hall_prev = motor->hall_state;
    motor->hall_state = (hall_c << 2) | (hall_b << 1) | hall_a;
}

// 根据霍尔信号计算转速
float BLDC_GetSpeedFromHall(BLDC_Motor_t *motor, float dt)
{
    // 每60度电角度产生一次霍尔状态变化
    // 转速 = 1 / (每转脉冲数 * 时间)
    if(dt <= 0)
        return 0.0f;
    
    // 电频率 = 1 / (dt * 6)  (6个霍尔状态为一个电周期)
    // 机械转速(RPM) = 电频率 * 60 / 极对数
    float electrical_freq = 1.0f / (dt * 6.0f);
    float speed_rpm = electrical_freq * 60.0f / motor->pole_pairs;
    
    return speed_rpm;
}

// 电流限制
void BLDC_CurrentLimit(BLDC_Motor_t *motor, float max_current)
{
    // 限制q轴电流参考值
    if(motor->foc.idq_ref.q > max_current)
        motor->foc.idq_ref.q = max_current;
    if(motor->foc.idq_ref.q < -max_current)
        motor->foc.idq_ref.q = -max_current;
    
    // 限制d轴电流参考值
    if(motor->foc.idq_ref.d > max_current)
        motor->foc.idq_ref.d = max_current;
    if(motor->foc.idq_ref.d < -max_current)
        motor->foc.idq_ref.d = -max_current;
}
