#include "main_control.h"
#include "tim.h"
#include "adc.h"
#include "math.h"
#include "string.h"

BLDC_Motor_t motor;
SystemState_t sys_state = SYS_INIT;
SystemConfig_t sys_config;
SystemMonitor_t sys_monitor;

void System_Init(void)
{
    sys_config.max_speed = MAX_SPEED;
    sys_config.max_current = 30.0f;
    sys_config.control_mode = 0;
    sys_config.comm_mode = COMMUTATION_FOC;
    sys_config.can_baudrate = 500000;
    sys_config.rs422_baudrate = 115200;
    
    sys_monitor.bus_voltage = 24.0f;
    sys_monitor.phase_current = 0.0f;
    sys_monitor.temperature = 25.0f;
    sys_monitor.uptime = 0;
    sys_monitor.error_code = 0;
    
    BLDC_Init(&motor);
    motor.mode = sys_config.comm_mode;
    
    CAN_Comm_Init();
    RS422_Init();
    I2C_Init();
    
    sys_state = SYS_READY;
}

void System_Start(void)
{
    if(sys_state == SYS_READY || sys_state == SYS_FAULT)
    {
        BLDC_Start(&motor);
        sys_state = SYS_RUNNING;
    }
}

void System_Stop(void)
{
    BLDC_Stop(&motor);
    sys_state = SYS_READY;
}

void System_Update(void)
{
    static uint32_t last_time = 0;
    uint32_t current_time;
    float dt;
    
    current_time = HAL_GetTick();
    dt = (current_time - last_time) / 1000.0f;
    if(dt <= 0 || dt > 0.1f)
        dt = 0.001f;
    
    System_CheckFault();
    
    if(sys_state == SYS_RUNNING)
    {
        BLDC_Update(&motor, dt);
    }
    
    sys_monitor.uptime = current_time;
    
    static uint32_t last_status_time = 0;
    if(current_time - last_status_time > 100)
    {
        float speed_rpm = motor.encoder.speed * 60.0f / (2 * 3.141592653589793f);
        CAN_Send_Status(speed_rpm, motor.Ia);
        last_status_time = current_time;
    }
    
    static uint32_t last_heartbeat_time = 0;
    if(current_time - last_heartbeat_time > 1000)
    {
        CAN_Send_Heartbeat();
        last_heartbeat_time = current_time;
    }
    
    last_time = current_time;
}

void System_CheckFault(void)
{
    uint32_t error = 0;
    
    if(motor.Ia > sys_config.max_current || 
       motor.Ib > sys_config.max_current || 
       motor.Ic > sys_config.max_current)
        error |= 0x0001;
    
    if(sys_monitor.bus_voltage < 18.0f)
        error |= 0x0002;
    if(sys_monitor.bus_voltage > 30.0f)
        error |= 0x0004;
    
    if(sys_monitor.temperature > 85.0f)
        error |= 0x0008;
    
    sys_monitor.error_code = error;
    
    if(error != 0 && sys_state == SYS_RUNNING)
    {
        System_Stop();
        sys_state = SYS_FAULT;
    }
}

void System_SaveConfig(void)
{
    uint8_t config_data[32];
    
    memcpy(&config_data[0], &sys_config.max_speed, 4);
    memcpy(&config_data[4], &sys_config.max_current, 4);
    config_data[8] = sys_config.control_mode;
    config_data[9] = sys_config.comm_mode;
    memcpy(&config_data[10], &sys_config.can_baudrate, 4);
    memcpy(&config_data[14], &sys_config.rs422_baudrate, 4);
    
    EEPROM_Write(0, config_data, 32);
}

void System_LoadConfig(void)
{
    uint8_t config_data[32];
    
    EEPROM_Read(0, config_data, 32);
    
    memcpy(&sys_config.max_speed, &config_data[0], 4);
    memcpy(&sys_config.max_current, &config_data[4], 4);
    sys_config.control_mode = config_data[8];
    sys_config.comm_mode = config_data[9];
    memcpy(&sys_config.can_baudrate, &config_data[10], 4);
    memcpy(&sys_config.rs422_baudrate, &config_data[14], 4);
}

void System_Calibrate(void)
{
    sys_state = SYS_CALIBRATING;
    Encoder_Calibrate(&motor.encoder);
    sys_state = SYS_READY;
}

void System_Heartbeat(void)
{
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
}

void Main_Loop(void)
{
    System_Init();
    
    while(1)
    {
        System_Update();
        
        if(rs422_comm.frame_ready)
        {
            RS422_Process_Frame();
        }
        
        HAL_Delay(1);
    }
}
