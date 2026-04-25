/* 
 * 说明：这是用户主程序文件
 * 请将此文件内容合并到CubeMX生成的main.c中
 * 或者将此文件添加到Keil项目中，并在main.c中调用Main_Loop()
 */

#include "main.h"
#include "main_control.h"
#include "bldc.h"
#include "encoder.h"
#include "can_comm.h"
#include "rs422_comm.h"
#include "i2c_dev.h"
#include "tim.h"
#include "spi.h"
#include "can.h"
#include "usart.h"
#include "i2c.h"
#include "adc.h"

// 外部变量声明
extern BLDC_Motor_t motor;
extern SystemState_t sys_state;

// 系统节拍定时器回调函数
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    static uint32_t tick_count = 0;
    
    // TIM6用于系统节拍 (假设1ms中断一次)
    if(htim->Instance == TIM6)
    {
        tick_count++;
        
        // 每1ms执行一次系统更新
        System_Update();
        
        // 心跳指示 (每500ms翻转一次LED)
        if(tick_count % 500 == 0)
        {
            System_Heartbeat();
        }
    }
    
    // 霍尔传感器定时器中断
    if(htim->Instance == TIM2 || htim->Instance == TIM3 || htim->Instance == TIM4)
    {
        // 霍尔状态变化，触发换相
        if(motor.mode == COMMUTATION_6STEP)
        {
            BLDC_ReadHallSensors(&motor);
            BLDC_SixStep_Commutation(&motor);
        }
    }
}

// CAN接收回调函数
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_Message_t msg;
    CAN_Receive_Callback(hcan, &msg);
}

// UART接收回调函数 (RS422)
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    static uint8_t rx_byte;
    
    if(huart->Instance == USART2)  // 根据实际UART修改
    {
        RS422_Process_Byte(rx_byte);
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    }
}

// ADC转换完成回调 (用于电流采样)
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    // 读取转换结果
    // phase_current.adc_raw[0] = HAL_ADC_GetValue(hadc);  // Ia
    // phase_current.ready = 1;
}

// main_user.c 示例代码
// 用户需要将此整合到CubeMX生成的 main.c 中
/*
int main(void)
{
    // HAL库初始化
    HAL_Init();
    
    // 系统时钟配置 (CubeMX生成)
    SystemClock_Config();
    
    // 外设初始化 (CubeMX生成)
    MX_GPIO_Init();
    MX_TIM1_Init();      // PWM定时器
    MX_TIM2_Init();      // 霍尔定时器
    MX_SPI1_Init();      // 编码器SPI
    MX_CAN1_Init();      // CAN通信
    MX_USART2_UART_Init(); // RS422 UART
    MX_I2C1_Init();      // I2C
    MX_ADC1_Init();      // 电流采样ADC
    MX_TIM6_Init();      // 系统节拍定时器
    
    // 用户代码开始
    System_Init();
    System_Start();
    
    // 启动系统定时器中断
    HAL_TIM_Base_Start_IT(&htim6);
    
    // 启动UART接收中断 (RS422)
    uint8_t rx_byte;
    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    
    // 启动ADC (电流采样)
    // HAL_ADC_Start_IT(&hadc1);
    
    // 主循环
    while (1)
    {
        Main_Loop();
    }
}
*/
