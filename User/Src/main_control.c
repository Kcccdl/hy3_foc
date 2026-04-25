#include "main_control.h"
#include "tim.h"
#include "adc.h"
#include "math.h"
#include "string.h"

// 全局变量定义
BLDC_Motor_t motor;
SystemState_t sys_state = SYS_INIT;
SystemConfig_t sys_config;
SystemMonitor_t sys_monitor;

// 系统初始化
void System_Init(void)
{
    // 初始化系统配置
    sys_config.max_speed = MAX_SPEED;
    sys_config.max_current = 30.0f;  // 30A
    sys_config.control_mode = 0;     // 速度模式
    sys_config.comm_mode = COMMUTATION_FOC;  // FOC模式
    sys_config.can_baudrate = 500000;      // 500kbps
    sys_config.rs422_baudrate = 115200;    // 115200bps
    
    // 初始化系统监控
    sys_monitor.bus_voltage = 24.0f;
    sys_monitor.phase_current = 0.0f;
    sys_monitor.temperature = 25.0f;
    sys_monitor.uptime = 0;
    sys_monitor.error_code = 0;
    
    // 初始化BLDC电机
    BLDC_Init(&motor);
    
    // 设置换相模式
    motor.mode = sys_config.comm_mode;
    
    // 初始化通信接口
    CAN_Comm_Init();
    RS422_Init();
    I2C_Init();
    
    // 启动系统定时器 (用于主循环计时)
    // HAL_TIM_Base_Start_IT(&htim6);  // 假设TIM6用于系统节拍
    
    // 更新系统状态
    sys_state = SYS_READY;
}

// 系统启动
void System_Start(void)
{
    if(sys_state == SYS_READY || sys_state == SYS_FAULT)
    {
        // 启动电机
        BLDC_Start(&motor);
        
        // 更新系统状态
        sys_state = SYS_RUNNING;
    }
}

// 系统停止
void System_Stop(void)
{
    // 停止电机
    BLDC_Stop(&motor);
    
    // 更新系统状态
    sys_state = SYS_READY;
}

// 系统更新 (主循环调用)
void System_Update(void)
{
    static uint32_t last_time = 0;
    uint32_t current_time;
    float dt;
    
    current_time = HAL_GetTick();
    
    // 计算时间间隔 (秒)
    dt = (current_time - last_time) / 1000.0f;
    if(dt <= 0 || dt > 0.1f)  // 限制dt范围
        dt = 0.001f;
    
    // 检查故障
    System_CheckFault();
    
    // 更新电机控制
    if(sys_state == SYS_RUNNING)
    {
        BLDC_Update(&motor, dt);
    }
    
    // 系统监控更新
    sys_monitor.uptime = current_time;
    
    // 周期性发送状态 (通过CAN和RS422)
    static uint32_t last_status_time = 0;
    if(current_time - last_status_time > 100)  // 每100ms发送一次
    {
        float speed_rpm = motor.encoder.speed * 60.0f / (2 * 3.141592653589793f);
        CAN_Send_Status(speed_rpm, motor.Ia);
        last_status_time = current_time;
    }
    
    // 心跳包
    static uint32_t last_heartbeat_time = 0;
    if(current_time - last_heartbeat_time > 1000)  // 每1秒发送一次
    {
        CAN_Send_Heartbeat();
        last_heartbeat_time = current_time;
    }
    
    last_time = current_time;
}

// 检查故障
void System_CheckFault(void)
{
    uint32_t error = 0;
    
    // 检查过流
    if(motor.Ia > sys_config.max_current || 
       motor.Ib > sys_config.max_current || 
       motor.Ic > sys_config.max_current)
    {
        error |= 0x0001;  // 过流故障
    }
    
    // 检查欠压/过压
    if(sys_monitor.bus_voltage < 18.0f)
        error |= 0x0002;  // 欠压
    if(sys_monitor.bus_voltage > 30.0f)
        error |= 0x0004;  // 过压
    
    // 检查过温
    if(sys_monitor.temperature > 85.0f)
        error |= 0x0008;  // 过温
    
    // 更新错误代码
    sys_monitor.error_code = error;
    
    // 如果有故障，停止电机
    if(error != 0 && sys_state == SYS_RUNNING)
    {
        System_Stop();
        sys_state = SYS_FAULT;
    }
}

// 保存配置到EEPROM
void System_SaveConfig(void)
{
    uint8_t config_data[32];
    
    // 将配置转换为字节流
    memcpy(&config_data[0], &sys_config.max_speed, 4);
    memcpy(&config_data[4], &sys_config.max_current, 4);
    config_data[8] = sys_config.control_mode;
    config_data[9] = sys_config.comm_mode;
    memcpy(&config_data[10], &sys_config.can_baudrate, 4);
    memcpy(&config_data[14], &sys_config.rs422_baudrate, 4);
    
    // 写入EEPROM
    EEPROM_Write(0, config_data, 32);
}

// 从EEPROM加载配置
void System_LoadConfig(void)
{
    uint8_t config_data[32];
    
    // 从EEPROM读取
    EEPROM_Read(0, config_data, 32);
    
    // 解析配置
    memcpy(&sys_config.max_speed, &config_data[0], 4);
    memcpy(&sys_config.max_current, &config_data[4], 4);
    sys_config.control_mode = config_data[8];
    sys_config.comm_mode = config_data[9];
    memcpy(&sys_config.can_baudrate, &config_data[10], 4);
    memcpy(&sys_config.rs422_baudrate, &config_data[14], 4);
}

// 系统校准 (编码器零点校准)
void System_Calibrate(void)
{
    sys_state = SYS_CALIBRATING;
    
    // 校准编码器零点
    Encoder_Calibrate(&motor.encoder);
    
    sys_state = SYS_READY;
}

// 心跳任务
void System_Heartbeat(void)
{
    // 可以通过LED闪烁或其他方式指示系统正常运行
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);  // 假设PB0接LED
}

// 主循环函数 (在main.c中调用)
void Main_Loop(void)
{
    // 系统初始化
    System_Init();
    
    // 主循环
    while(1)
    {
        // 系统更新
        System_Update();
        
        // 处理RS422接收 (如果使用了中断，这里可以处理接收到的帧)
        if(rs422_comm.frame_ready)
        {
            RS422_Process_Frame();
        }
        
        // 延时或进入低功耗模式
        HAL_Delay(1);  // 1ms延时，实际可以根据需要调整
    }
}
