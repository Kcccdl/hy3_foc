#ifndef __MAIN_CONTROL_H_
#define __MAIN_CONTROL_H_

#include "stm32f4xx_hal.h"
#include "bldc.h"
#include "foc.h"
#include "pid.h"
#include "encoder.h"
#include "can_comm.h"
#include "rs422_comm.h"
#include "i2c_dev.h"

// 系统状态定义
typedef enum
{
    SYS_INIT = 0,          // 初始化;
    SYS_READY = 1,         // 就绪;
    SYS_RUNNING = 2,       // 运行中;
    SYS_FAULT = 3,         // 故障;
    SYS_CALIBRATING = 4    // 校准中;
} SystemState_t;

// 系统配置结构体
typedef struct
{
    float max_speed;          // 最大速度 (RPM);
    float max_current;        // 最大电流 (A);
    uint8_t control_mode;     // 控制模式: 0=速度,1=转矩;
    uint8_t comm_mode;        // 换相模式: 0=六步,1=FOC;
    uint32_t can_baudrate;    // CAN波特率;
    uint32_t rs422_baudrate;  // RS422波特率;
} SystemConfig_t;

// 系统监控结构体
typedef struct
{
    float bus_voltage;       // 母线电压;
    float phase_current;     // 相电流;
    float temperature;       // 温度;
    uint32_t uptime;        // 运行时间 (ms);
    uint32_t error_code;    // 错误代码;
} SystemMonitor_t;

// 全局变量声明
extern BLDC_Motor_t motor;
extern SystemState_t sys_state;
extern SystemConfig_t sys_config;
extern SystemMonitor_t sys_monitor;

// 函数声明
void System_Init(void);
void System_Start(void);
void System_Stop(void);
void System_Update(void);
void System_CheckFault(void);
void System_SaveConfig(void);
void System_LoadConfig(void);
void System_Calibrate(void);
void System_Heartbeat(void);
void Main_Loop(void);

#endif /* __MAIN_CONTROL_H_ */
