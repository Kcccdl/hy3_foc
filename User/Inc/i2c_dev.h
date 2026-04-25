#ifndef __I2C_DEV_H
#define __I2C_DEV_H

#include "stm32f4xx_hal.h"
#include "i2c.h"

// I2C设备地址定义 (根据实际设备修改)
#define I2C_DEV_ADDR_EEPROM   0xA0    // EEPROM设备地址 (如24C02)
#define I2C_DEV_ADDR_SENSOR   0x48    // 温度传感器 (如LM75)
#define I2C_DEV_ADDR_ADC      0x48    // I2C ADC (如ADS1115)

// I2C通信超时时间
#define I2C_TIMEOUT 100

// I2C设备结构体
typedef struct
{
    uint8_t initialized;      // 初始化标志
    uint32_t error_count;     // 错误计数
} I2C_Device_t;

// 函数声明
void I2C_Init(void);
HAL_StatusTypeDef I2C_Write_Byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t data);
HAL_StatusTypeDef I2C_Read_Byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data);
HAL_StatusTypeDef I2C_Write_Bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
HAL_StatusTypeDef I2C_Read_Bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
HAL_StatusTypeDef I2C_Scan_Device(uint8_t dev_addr);
void I2C_Scan_All(void);

// EEPROM操作函数 (示例)
HAL_StatusTypeDef EEPROM_Write(uint16_t addr, uint8_t *data, uint16_t len);
HAL_StatusTypeDef EEPROM_Read(uint16_t addr, uint8_t *data, uint16_t len);

// 温度传感器操作函数 (示例)
float Temperature_Read(void);

extern I2C_Device_t i2c_dev;
extern I2C_HandleTypeDef hi2c1;  // 根据实际配置修改

#endif /* __I2C_DEV_H */
