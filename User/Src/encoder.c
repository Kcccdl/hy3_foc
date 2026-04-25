#include "encoder.h"
#include "spi.h"
#include "math.h"

// SPI句柄使用CubeMX生成的hspi1
extern SPI_HandleTypeDef hspi1;

// 角度转换宏
#define DEG_TO_RAD (3.141592653589793f / 180.0f)
#define RAD_TO_DEG (180.0f / 3.141592653589793f)

// 编码器初始化
void Encoder_Init(Encoder_t *encoder)
{
    encoder->angle = 0.0f;
    encoder->prev_angle = 0.0f;
    encoder->speed = 0.0f;
    encoder->angle_raw = 0.0f;
    encoder->encoder_value = 0;
    encoder->encoder_prev = 0;
    encoder->angle_offset = 0.0f;
    encoder->timestamp = 0;
    encoder->prev_timestamp = 0;
}

// 读取KTM5910磁编码器角度
void KTM5910_ReadAngle(Encoder_t *encoder)
{
    uint8_t tx_buf[2];
    uint8_t rx_buf[2];
    uint16_t raw_angle;
    
    // KTM5910读取角度命令 (具体命令需参考KTM5910 datasheet)
    tx_buf[0] = 0x00;  // 读取命令
    tx_buf[1] = 0x00;  // 地址或虚拟数据
    
    // 拉低CS片选
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);  // 根据实际CS引脚修改
    
    // SPI传输
    HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, 2, HAL_MAX_DELAY);
    
    // 拉高CS片选
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    
    // 解析角度数据 (假设14位分辨率，0-16383对应0-360度)
    raw_angle = ((uint16_t)rx_buf[0] << 8) | rx_buf[1];
    raw_angle = raw_angle & 0x3FFF;  // 取低14位
    
    encoder->encoder_value = raw_angle;
    
    // 转换为角度 (0-360度)
    encoder->angle_raw = (float)raw_angle * 360.0f / 16384.0f;
    
    // 转换为弧度，并加上偏移校准
    encoder->angle = encoder->angle_raw * DEG_TO_RAD + encoder->angle_offset;
    
    // 归一化到0-2PI
    while(encoder->angle >= 2 * 3.141592653589793f)
        encoder->angle -= 2 * 3.141592653589793f;
    while(encoder->angle < 0)
        encoder->angle += 2 * 3.141592653589793f;
}

// 获取当前角度
float Encoder_GetAngle(Encoder_t *encoder)
{
    return encoder->angle;
}

// 计算转速 (rad/s)
float Encoder_GetSpeed(Encoder_t *encoder, float dt)
{
    float angle_diff;
    
    // 计算角度差，处理过零问题
    angle_diff = encoder->angle - encoder->prev_angle;
    
    // 处理角度越过360度/0度的情况
    if(angle_diff > 3.141592653589793f)
        angle_diff -= 2 * 3.141592653589793f;
    else if(angle_diff < -3.141592653589793f)
        angle_diff += 2 * 3.141592653589793f;
    
    // 计算角速度
    if(dt > 0)
        encoder->speed = angle_diff / dt;
    else
        encoder->speed = 0.0f;
    
    return encoder->speed;
}

// 编码器更新函数
void Encoder_Update(Encoder_t *encoder, float dt)
{
    // 保存上一次角度
    encoder->prev_angle = encoder->angle;
    
    // 读取新的角度
    KTM5910_ReadAngle(encoder);
    
    // 计算转速
    Encoder_GetSpeed(encoder, dt);
}

// 设置角度偏移 (用于校准)
void Encoder_SetOffset(Encoder_t *encoder, float offset)
{
    encoder->angle_offset = offset;
}

// 编码器校准函数
void Encoder_Calibrate(Encoder_t *encoder)
{
    // 读取当前角度作为零点参考
    KTM5910_ReadAngle(encoder);
    
    // 设置偏移使得当前角度为0
    encoder->angle_offset = -encoder->angle;
    
    // 重新读取验证
    KTM5910_ReadAngle(encoder);
}

// SPI读取16位数据 (通用函数)
uint16_t SPI_Read_16Bits(uint8_t reg)
{
    uint8_t tx_buf[3];
    uint8_t rx_buf[3];
    uint16_t data;
    
    tx_buf[0] = reg | 0x80;  // 读命令 (最高位为1)
    tx_buf[1] = 0x00;
    tx_buf[2] = 0x00;
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, 3, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    
    data = ((uint16_t)rx_buf[1] << 8) | rx_buf[2];
    return data;
}

// SPI写入16位数据 (通用函数)
void SPI_Write_16Bits(uint8_t reg, uint16_t data)
{
    uint8_t tx_buf[3];
    
    tx_buf[0] = reg & 0x7F;  // 写命令 (最高位为0)
    tx_buf[1] = (data >> 8) & 0xFF;
    tx_buf[2] = data & 0xFF;
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, tx_buf, 3, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}
