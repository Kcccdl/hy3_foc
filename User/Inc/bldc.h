#ifndef __BLDC_H
#define __BLDC_H

#include "stm32f4xx_hal.h"
#include "pid.h"
#include "foc.h"
#include "encoder.h"

// 电机参数定义
#define POLE_PAIRS 4          // 电机极对数 (根据实际电机修改)
#define MAX_SPEED 3000        // 最大转速 (RPM)
#define PWM_PERIOD 4200       // PWM周期值 (与CubeMX配置一致)

// 霍尔传感器状态
#define HALL_STATE_1 0x05     // 101
#define HALL_STATE_2 0x04     // 100
#define HALL_STATE_3 0x06     // 110
#define HALL_STATE_4 0x02     // 010
#define HALL_STATE_5 0x03     // 011
#define HALL_STATE_6 0x01     // 001

// 换相模式
typedef enum
{
    COMMUTATION_6STEP = 0,    // 六步换相
    COMMUTATION_FOC = 1       // FOC控制
} CommutationMode_t;

// 电机运行状态
typedef enum
{
    MOTOR_STOP = 0,           // 停止
    MOTOR_RUN = 1,            // 运行
    MOTOR_FAULT = 2           // 故障
} MotorState_t;

// BLDC电机结构体
typedef struct
{
    // 电机基本参数
    uint8_t pole_pairs;       // 极对数
    float max_speed;           // 最大转速 (RPM)
    
    // 电流采样值 (需要经过ADC采样)
    float Ia;                  // A相电流
    float Ib;                  // B相电流
    float Ic;                  // C相电流
    
    // 霍尔传感器状态
    uint8_t hall_state;       // 当前霍尔状态 (3位)
    uint8_t hall_prev;        // 上一次霍尔状态
    uint32_t hall_timestamp;  // 霍尔时间戳
    float hall_speed;         // 霍尔测速
    
    // PWM相关
    uint32_t pwm_period;      // PWM周期
    float duty_cycle;         // 占空比 (0.0 - 1.0)
    
    // 控制模式
    CommutationMode_t mode;   // 换相模式
    MotorState_t state;       // 电机状态
    
    // 控制器
    PID_t speed_pid;          // 速度PID
    PID_t current_pid;        // 电流PID (q轴)
    FOC_t foc;                // FOC控制器
    Encoder_t encoder;        // 磁编码器
    
    // 目标值
    float target_speed;       // 目标速度 (RPM)
    float target_current;     // 目标电流 (A)
    
} BLDC_Motor_t;

// 函数声明
void BLDC_Init(BLDC_Motor_t *motor);
void BLDC_SetSpeed(BLDC_Motor_t *motor, float speed_rpm);
void BLDC_SetCurrent(BLDC_Motor_t *motor, float current_a);
void BLDC_Start(BLDC_Motor_t *motor);
void BLDC_Stop(BLDC_Motor_t *motor);
void BLDC_SixStep_Commutation(BLDC_Motor_t *motor);
void BLDC_FOC_Control(BLDC_Motor_t *motor, float dt);
void BLDC_Update(BLDC_Motor_t *motor, float dt);
void BLDC_SetPWM(float duty_u, float duty_v, float duty_w);
void BLDC_ReadHallSensors(BLDC_Motor_t *motor);
float BLDC_GetSpeedFromHall(BLDC_Motor_t *motor, float dt);
void BLDC_CurrentLimit(BLDC_Motor_t *motor, float max_current);

#endif /* __BLDC_H */
