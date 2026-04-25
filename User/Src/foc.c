#include "foc.h"
#include "math.h"

#define SQRT3 1.732050807568877f
#define TWO_PI 6.283185307179586f

// FOC初始化函数
void FOC_Init(FOC_t *foc)
{
    // 初始化所有参数为0
    foc->iab.alpha = 0.0f;
    foc->iab.beta = 0.0f;
    
    foc->idq.d = 0.0f;
    foc->idq.q = 0.0f;
    
    foc->vdq.d = 0.0f;
    foc->vdq.q = 0.0f;
    
    foc->idq_ref.d = 0.0f;
    foc->idq_ref.q = 0.0f;
    
    foc->rotor.angle = 0.0f;
    foc->rotor.speed = 0.0f;
    foc->rotor.theta = 0.0f;
    
    foc->svpwm.Udc = 24.0f;           // 直流母线电压24V
    foc->svpwm.max_modulation = 0.95f; // 最大调制比95%
    
    foc->pwm.Ta = 0.0f;
    foc->pwm.Tb = 0.0f;
    foc->pwm.Tc = 0.0f;
    
    foc->theta_e = 0.0f;
}

// Clark变换：三相静止坐标系(abc) -> 两相静止坐标系(alpha-beta)
void Clark_Transform(float Ia, float Ib, float Ic, AlphaBeta_t *i_ab)
{
    i_ab->alpha = Ia;
    i_ab->beta = (Ia + 2.0f * Ib) / SQRT3;
}

// Park变换：两相静止坐标系(alpha-beta) -> 两相旋转坐标系(d-q)
void Park_Transform(AlphaBeta_t *i_ab, float theta, DQ_t *idq)
{
    float cos_theta = cosf(theta);
    float sin_theta = sinf(theta);
    
    idq->d = i_ab->alpha * cos_theta + i_ab->beta * sin_theta;
    idq->q = -i_ab->alpha * sin_theta + i_ab->beta * cos_theta;
}

// 反Park变换：两相旋转坐标系(d-q) -> 两相静止坐标系(alpha-beta)
void Inverse_Park_Transform(DQ_t *vdq, float theta, AlphaBeta_t *v_ab)
{
    float cos_theta = cosf(theta);
    float sin_theta = sinf(theta);
    
    v_ab->alpha = vdq->d * cos_theta - vdq->q * sin_theta;
    v_ab->beta = vdq->d * sin_theta + vdq->q * cos_theta;
}

// SVPWM生成函数
void SVPWM_Generate(AlphaBeta_t *v_ab, SVPWM_t *svpwm, PWM_Output_t *pwm)
{
    float Valpha = v_ab->alpha;
    float Vbeta = v_ab->beta;
    float Udc = svpwm->Udc;
    
    // 计算扇区
    float Va = Vbeta;
    float Vb = (SQRT3 * Valpha - Vbeta) / 2.0f;
    float Vc = (-SQRT3 * Valpha - Vbeta) / 2.0f;
    
    int sector = 0;
    if (Va > 0) sector += 1;
    if (Vb > 0) sector += 2;
    if (Vc > 0) sector += 4;
    
    // 计算相邻矢量的作用时间
    float T1, T2, T0;
    float X = SQRT3 * Vbeta * 2.0f / Udc;
    float Y = (3.0f * Valpha + SQRT3 * Vbeta) / Udc;
    float Z = (-3.0f * Valpha + SQRT3 * Vbeta) / Udc;
    
    switch(sector)
    {
        case 1: T1 = Z; T2 = Y; break;
        case 2: T1 = Y; T2 = -X; break;
        case 3: T1 = -Z; T2 = X; break;
        case 4: T1 = -X; T2 = Z; break;
        case 5: T1 = X; T2 = -Y; break;
        case 6: T1 = -Y; T2 = -Z; break;
        default: T1 = 0; T2 = 0; break;
    }
    
    // 归一化到[0,1]
    T1 = T1 / 2.0f;
    T2 = T2 / 2.0f;
    
    // 限制到最大调制比
    float T_sum = T1 + T2;
    if(T_sum > svpwm->max_modulation)
    {
        T1 = T1 * svpwm->max_modulation / T_sum;
        T2 = T2 * svpwm->max_modulation / T_sum;
    }
    
    T0 = 1.0f - T1 - T2;
    
    // 计算三相占空比
    switch(sector)
    {
        case 1: pwm->Ta = T1 + T2 + T0/2; pwm->Tb = T2 + T0/2; pwm->Tc = T0/2; break;
        case 2: pwm->Ta = T1 + T0/2; pwm->Tb = T1 + T2 + T0/2; pwm->Tc = T0/2; break;
        case 3: pwm->Ta = T0/2; pwm->Tb = T1 + T2 + T0/2; pwm->Tc = T2 + T0/2; break;
        case 4: pwm->Ta = T0/2; pwm->Tb = T1 + T0/2; pwm->Tc = T1 + T2 + T0/2; break;
        case 5: pwm->Ta = T2 + T0/2; pwm->Tb = T0/2; pwm->Tc = T1 + T2 + T0/2; break;
        case 6: pwm->Ta = T1 + T2 + T0/2; pwm->Tb = T0/2; pwm->Tc = T1 + T0/2; break;
        default: pwm->Ta = 0.5f; pwm->Tb = 0.5f; pwm->Tc = 0.5f; break;
    }
}

// FOC主更新函数
void FOC_Update(FOC_t *foc, float Ia, float Ib, float Ic, float theta_mech, float Ts)
{
    // 更新转子机械角度和电角度
    foc->rotor.angle = theta_mech;
    foc->theta_e = theta_mech;  // 如果是多对极电机，需要乘以极对数
    
    // Clark变换
    Clark_Transform(Ia, Ib, Ic, &foc->iab);
    
    // Park变换
    Park_Transform(&foc->iab, foc->theta_e, &foc->idq);
    
    // 这里应该加入电流PID控制器
    // foc->vdq.d = PID_d(foc->idq_ref.d - foc->idq.d);
    // foc->vdq.q = PID_q(foc->idq_ref.q - foc->idq.q);
    
    // 反Park变换
    AlphaBeta_t v_ab;
    Inverse_Park_Transform(&foc->vdq, foc->theta_e, &v_ab);
    
    // SVPWM生成
    SVPWM_Generate(&v_ab, &foc->svpwm, &foc->pwm);
}
