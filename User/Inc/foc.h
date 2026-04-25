#ifndef __FOC_H
#define __FOC_H

#include "stm32f4xx_hal.h"
#include "math.h"

// FOC相关结构体定义
typedef struct
{
    float alpha;    // Clark变换后的alpha轴分量
    float beta;     // Clark变换后的beta轴分量
} AlphaBeta_t;

typedef struct
{
    float d;        // Park变换后的d轴分量
    float q;        // Park变换后的q轴分量
} DQ_t;

typedef struct
{
    float angle;     // 转子角度 (弧度)
    float speed;     // 转子速度 (rad/s)
    float theta;     // 电角度 (弧度)
} Rotor_t;

typedef struct
{
    float Ta;        // 三相PWM占空比 A相
    float Tb;        // 三相PWM占空比 B相
    float Tc;        // 三相PWM占空比 C相
} PWM_Output_t;

typedef struct
{
    float Ud;        // d轴电压指令
    float Uq;        // q轴电压指令
    float Udc;       // 直流母线电压
    float max_modulation;  // 最大调制比
} SVPWM_t;

// FOC控制器结构体
typedef struct
{
    AlphaBeta_t iab;    // alpha-beta坐标系下的电流
    DQ_t idq;           // dq坐标系下的电流
    DQ_t vdq;           // dq坐标系下的电压
    DQ_t idq_ref;       // dq坐标系下的电流参考值
    Rotor_t rotor;      // 转子状态
    SVPWM_t svpwm;      // SVPWM模块
    PWM_Output_t pwm;   // PWM输出
    float theta_e;      // 电角度
} FOC_t;

// 函数声明
void FOC_Init(FOC_t *foc);
void Clark_Transform(float Ia, float Ib, float Ic, AlphaBeta_t *iab);
void Park_Transform(AlphaBeta_t *iab, float theta, DQ_t *idq);
void Inverse_Park_Transform(DQ_t *vdq, float theta, AlphaBeta_t *vab);
void SVPWM_Generate(AlphaBeta_t *vab, SVPWM_t *svpwm, PWM_Output_t *pwm);
void FOC_Update(FOC_t *foc, float Ia, float Ib, float Ic, float theta_mech, float Ts);

#endif /* __FOC_H */
