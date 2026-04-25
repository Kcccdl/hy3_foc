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

extern BLDC_Motor_t motor;
extern SystemState_t sys_state;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    static uint32_t tick_count = 0;
    
    if(htim->Instance == TIM6)
    {
        tick_count++;
        System_Update();
        
        if(tick_count % 500 == 0)
        {
            System_Heartbeat();
        }
    }
    
    if(htim->Instance == TIM2 || htim->Instance == TIM3 || htim->Instance == TIM4)
    {
        if(motor.mode == COMMUTATION_6STEP)
        {
            BLDC_ReadHallSensors(&motor);
            BLDC_SixStep_Commutation(&motor);
        }
    }
}

//void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
//{
//    CAN_Message_t msg;
//    CAN_Receive_Callback(hcan, &msg);
//}

//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//    static uint8_t rx_byte;
//    
//    if(huart->Instance == USART2)
//    {
//        RS422_Process_Byte(rx_byte);
//        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
//    }
//}

//void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
//{
//    // phase_current.adc_raw[0] = HAL_ADC_GetValue(hadc);
//    // phase_current.ready = 1;
//}

/*
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    
    MX_GPIO_Init();
    MX_TIM1_Init();
    MX_TIM2_Init();
    MX_SPI1_Init();
    MX_CAN1_Init();
    MX_USART2_UART_Init();
    MX_I2C1_Init();
    MX_ADC1_Init();
    MX_TIM6_Init();
    
    System_Init();
    System_Start();
    
    HAL_TIM_Base_Start_IT(&htim6);
    
    uint8_t rx_byte;
    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    
    while (1)
    {
        Main_Loop();
    }
}
*/
