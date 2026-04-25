#include "i2c_dev.h"
#include "string.h"

I2C_HandleTypeDef hi2c1;  // CubeMX生成的I2C句柄
I2C_Device_t i2c_dev = {0};

// I2C初始化
void I2C_Init(void)
{
    i2c_dev.initialized = 1;
    i2c_dev.error_count = 0;
}

// I2C写单个字节
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

// I2C读单个字节
HAL_StatusTypeDef I2C_Read_Byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data)
{
    HAL_StatusTypeDef status;
    
    // 先发送寄存器地址
    status = HAL_I2C_Master_Transmit(&hi2c1, dev_addr, &reg_addr, 1, I2C_TIMEOUT);
    if(status != HAL_OK)
    {
        i2c_dev.error_count++;
        return status;
    }
    
    // 再读取数据
    status = HAL_I2C_Master_Receive(&hi2c1, dev_addr, data, 1, I2C_TIMEOUT);
    if(status != HAL_OK)
        i2c_dev.error_count++;
    
    return status;
}

// I2C写多个字节
HAL_StatusTypeDef I2C_Write_Bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    HAL_StatusTypeDef status;
    uint8_t *tx_buffer;
    
    // 分配缓冲区 (寄存器地址 + 数据)
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

// I2C读多个字节
HAL_StatusTypeDef I2C_Read_Bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    HAL_StatusTypeDef status;
    
    // 先发送寄存器地址
    status = HAL_I2C_Master_Transmit(&hi2c1, dev_addr, &reg_addr, 1, I2C_TIMEOUT);
    if(status != HAL_OK)
    {
        i2c_dev.error_count++;
        return status;
    }
    
    // 读取多个字节
    status = HAL_I2C_Master_Receive(&hi2c1, dev_addr, data, len, I2C_TIMEOUT);
    if(status != HAL_OK)
        i2c_dev.error_count++;
    
    return status;
}

// 扫描单个I2C设备是否在线
HAL_StatusTypeDef I2C_Scan_Device(uint8_t dev_addr)
{
    return HAL_I2C_Master_Transmit(&hi2c1, dev_addr, NULL, 0, I2C_TIMEOUT);
}

// 扫描所有I2C设备 (7位地址范围: 0x08-0x77)
void I2C_Scan_All(void)
{
    uint8_t addr;
    HAL_StatusTypeDef status;
    
    for(addr = 0x08; addr <= 0x77; addr++)
    {
        status = I2C_Scan_Device(addr << 1);  // HAL库地址需要左移1位
        if(status == HAL_OK)
        {
            // 设备存在于该地址
            // 可以通过串口打印或其他方式输出
        }
    }
}

// EEPROM写操作 (以24C02为例，256字节)
HAL_StatusTypeDef EEPROM_Write(uint16_t addr, uint8_t *data, uint16_t len)
{
    uint8_t dev_addr = I2C_DEV_ADDR_EEPROM;  // 0xA0
    HAL_StatusTypeDef status;
    
    // 24C02写操作: 设备地址 + 内存地址 + 数据
    // 这里简化，实际需要处理跨页写
    status = I2C_Write_Bytes(dev_addr, (uint8_t)addr, data, len);
    
    // EEPROM写周期需要延时 (典型5ms)
    HAL_Delay(5);
    
    return status;
}

// EEPROM读操作
HAL_StatusTypeDef EEPROM_Read(uint16_t addr, uint8_t *data, uint16_t len)
{
    uint8_t dev_addr = I2C_DEV_ADDR_EEPROM;  // 0xA0
    
    return I2C_Read_Bytes(dev_addr, (uint8_t)addr, data, len);
}

// 读取温度传感器 (以LM75为例)
float Temperature_Read(void)
{
    uint8_t dev_addr = I2C_DEV_ADDR_SENSOR;
    uint8_t data[2];
    int16_t temp_raw;
    float temperature;
    
    // LM75温度寄存器地址为0x00
    if(I2C_Read_Bytes(dev_addr, 0x00, data, 2) == HAL_OK)
    {
        temp_raw = ((int16_t)data[0] << 8) | data[1];
        temp_raw >>= 5;  // LM75数据格式: 11位，左对齐
        
        // 分辨率为0.125度
        temperature = temp_raw * 0.125f;
        
        return temperature;
    }
    
    return -999.0f;  // 错误返回值
}
