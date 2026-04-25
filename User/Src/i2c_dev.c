#include "i2c_dev.h"
#include "string.h"
#include <stdlib.h>

I2C_Device_t i2c_dev = {0};
extern I2C_HandleTypeDef hi2c1;

void I2C_Init(void)
{
    i2c_dev.initialized = 1;
    i2c_dev.error_count = 0;
}

HAL_StatusTypeDef I2C_Write_Byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t data)
{
    HAL_StatusTypeDef status;
    uint8_t tx_data[2];
    
    tx_data[0] = reg_addr;
    tx_data[1] = data;
    
    status = HAL_I2C_Master_Transmit(&hi2c1, dev_addr, tx_data, 2, I2C_TIMEOUT);
    
    if(status != HAL_OK)
        i2c_dev.error_count++;
    
    return status;
}

HAL_StatusTypeDef I2C_Read_Byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data)
{
    HAL_StatusTypeDef status;
    
    status = HAL_I2C_Master_Transmit(&hi2c1, dev_addr, &reg_addr, 1, I2C_TIMEOUT);
    if(status != HAL_OK)
    {
        i2c_dev.error_count++;
        return status;
    }
    
    status = HAL_I2C_Master_Receive(&hi2c1, dev_addr, data, 1, I2C_TIMEOUT);
    if(status != HAL_OK)
        i2c_dev.error_count++;
    
    return status;
}

HAL_StatusTypeDef I2C_Write_Bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    HAL_StatusTypeDef status;
    uint8_t *tx_buffer;
    
    tx_buffer = (uint8_t*)malloc(len + 1);
    if(tx_buffer == NULL)
        return HAL_ERROR;
    
    tx_buffer[0] = reg_addr;
    memcpy(&tx_buffer[1], data, len);
    
    status = HAL_I2C_Master_Transmit(&hi2c1, dev_addr, tx_buffer, len + 1, I2C_TIMEOUT);
    
    free(tx_buffer);
    
    if(status != HAL_OK)
        i2c_dev.error_count++;
    
    return status;
}

HAL_StatusTypeDef I2C_Read_Bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    HAL_StatusTypeDef status;
    
    status = HAL_I2C_Master_Transmit(&hi2c1, dev_addr, &reg_addr, 1, I2C_TIMEOUT);
    if(status != HAL_OK)
    {
        i2c_dev.error_count++;
        return status;
    }
    
    status = HAL_I2C_Master_Receive(&hi2c1, dev_addr, data, len, I2C_TIMEOUT);
    if(status != HAL_OK)
        i2c_dev.error_count++;
    
    return status;
}

HAL_StatusTypeDef I2C_Scan_Device(uint8_t dev_addr)
{
    return HAL_I2C_Master_Transmit(&hi2c1, dev_addr, NULL, 0, I2C_TIMEOUT);
}

void I2C_Scan_All(void)
{
    uint8_t addr;
    HAL_StatusTypeDef status;
    
    for(addr = 0x08; addr <= 0x77; addr++)
    {
        status = I2C_Scan_Device(addr << 1);
        if(status == HAL_OK)
        {
            // Device found
        }
    }
}

HAL_StatusTypeDef EEPROM_Write(uint16_t addr, uint8_t *data, uint16_t len)
{
    uint8_t dev_addr = I2C_DEV_ADDR_EEPROM;
    HAL_StatusTypeDef status;
    
    status = I2C_Write_Bytes(dev_addr, (uint8_t)addr, data, len);
    
    HAL_Delay(5);
    
    return status;
}

HAL_StatusTypeDef EEPROM_Read(uint16_t addr, uint8_t *data, uint16_t len)
{
    uint8_t dev_addr = I2C_DEV_ADDR_EEPROM;
    return I2C_Read_Bytes(dev_addr, (uint8_t)addr, data, len);
}

float Temperature_Read(void)
{
    uint8_t dev_addr = I2C_DEV_ADDR_SENSOR;
    uint8_t data[2];
    int16_t temp_raw;
    float temperature;
    
    if(I2C_Read_Bytes(dev_addr, 0x00, data, 2) == HAL_OK)
    {
        temp_raw = ((int16_t)data[0] << 8) | data[1];
        temp_raw >>= 5;
        temperature = temp_raw * 0.125f;
        return temperature;
    }
    
    return -999.0f;
}
