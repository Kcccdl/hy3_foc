#include "encoder.h"
#include "spi.h"
#include "math.h"
#include <stdint.h>

#define DEG_TO_RAD (3.141592653589793f / 180.0f)
#define RAD_TO_DEG (180.0f / 3.141592653589793f)

extern SPI_HandleTypeDef hspi1;

// ---------- KTM5910 SPI 指令定义 ----------
#define KTM_CMD_READ_ANGLE   0x80000000  // 指令3: 读取角度+状态+CRC
#define KTM_CMD_WRITE_REG    0x00000000  // 指令1 基址
#define KTM_CMD_READ_REG     0x40000000  // 指令2 基址

#define KTM_WRITE_CMD(addr, data) (((uint32_t)(addr & 0x7F) << 16) | ((data) & 0xFFFF))
#define KTM_READ_CMD(addr)       (KTM_CMD_READ_REG | ((uint32_t)(addr & 0x7F) << 16))

// ---------- CRC8 表 (多项式 X^8 + X^2 + X + 1) ----------
static const uint8_t CRC8_TABLE[256] = {
    0x00,0x07,0x0e,0x09,0x1c,0x1b,0x12,0x15,0x38,0x3f,0x36,0x31,0x24,0x23,0x2a,0x2d,
    0x70,0x77,0x7e,0x79,0x6c,0x6b,0x62,0x65,0x48,0x4f,0x46,0x41,0x54,0x53,0x5a,0x5d,
    0xe0,0xe7,0xee,0xe9,0xfc,0xfb,0xf2,0xf5,0xd8,0xdf,0xd6,0xd1,0xc4,0xc3,0xca,0xcd,
    0x90,0x97,0x9e,0x99,0x8c,0x8b,0x82,0x85,0xa8,0xaf,0xa6,0xa1,0xb4,0xb3,0xba,0xbd,
    0xc7,0xc0,0xc9,0xce,0xdb,0xdc,0xd5,0xd2,0xff,0xf8,0xf1,0xf6,0xe3,0xe4,0xed,0xea,
    0xb7,0xb0,0xb9,0xbe,0xab,0xac,0xa5,0xa2,0x8f,0x88,0x81,0x86,0x93,0x94,0x9d,0x9a,
    0x27,0x20,0x29,0x2e,0x3b,0x3c,0x35,0x32,0x1f,0x18,0x11,0x16,0x03,0x04,0x0d,0x0a,
    0x57,0x50,0x59,0x5e,0x4b,0x4c,0x45,0x42,0x6f,0x68,0x61,0x66,0x73,0x74,0x7d,0x7a,
    0x89,0x8e,0x87,0x80,0x95,0x92,0x9b,0x9c,0xb1,0xb6,0xbf,0xb8,0xad,0xaa,0xa3,0xa4,
    0xf9,0xfe,0xf7,0xf0,0xe5,0xe2,0xeb,0xec,0xc1,0xc6,0xcf,0xc8,0xdd,0xda,0xd3,0xd4,
    0x69,0x6e,0x67,0x60,0x75,0x72,0x7b,0x7c,0x51,0x56,0x5f,0x58,0x4d,0x4a,0x43,0x44,
    0x19,0x1e,0x17,0x10,0x05,0x02,0x0b,0x0c,0x21,0x26,0x2f,0x28,0x3d,0x3a,0x33,0x34,
    0x4e,0x49,0x40,0x47,0x52,0x55,0x5c,0x5b,0x76,0x71,0x78,0x7f,0x6a,0x6d,0x64,0x63,
    0x3e,0x39,0x30,0x37,0x22,0x25,0x2c,0x2b,0x06,0x01,0x08,0x0f,0x1a,0x1d,0x14,0x13,
    0xae,0xa9,0xa0,0xa7,0xb2,0xb5,0xbc,0xbb,0x96,0x91,0x98,0x9f,0x8a,0x8d,0x84,0x83,
    0xde,0xd9,0xd0,0xd7,0xc2,0xc5,0xcc,0xcb,0xe6,0xe1,0xe8,0xef,0xfa,0xfd,0xf4,0xf3
};

// ---------- 底层 SPI 传输 (4 字节) ----------
static void KTM_SPI_Transmit32(uint32_t tx_data, uint32_t *rx_data)
{
    uint8_t tx[4] = {
        (tx_data >> 24) & 0xFF,
        (tx_data >> 16) & 0xFF,
        (tx_data >> 8) & 0xFF,
        tx_data & 0xFF
    };
    uint8_t rx[4];

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi1, tx, rx, 4, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

    if (rx_data) {
        *rx_data = ((uint32_t)rx[0] << 24) |
                   ((uint32_t)rx[1] << 16) |
                   ((uint32_t)rx[2] << 8)  |
                   rx[3];
    }
}

// ---------- 寄存器读写 (修正后的 KTM_ReadReg) ----------
void KTM_WriteReg(uint8_t addr, uint16_t data)
{
    uint32_t cmd = KTM_WRITE_CMD(addr, data);
    KTM_SPI_Transmit32(cmd, NULL);
    KTM_SPI_Transmit32(KTM_CMD_READ_ANGLE, NULL);
}

uint16_t KTM_ReadReg(uint8_t addr)
{
    uint32_t cmd = KTM_READ_CMD(addr);
    uint32_t rx;
    KTM_SPI_Transmit32(cmd, NULL);
    KTM_SPI_Transmit32(KTM_CMD_READ_ANGLE, &rx);
    // 修正：寄存器数据在 bit[31:24]
    return (uint16_t)((rx >> 24) & 0xFF);
}

// ---------- CRC8 计算 ----------
static uint8_t KTM_CRC8(const uint8_t *data, size_t len)
{
    uint8_t crc = 0x00;
    for (size_t i = 0; i < len; i++) {
        crc = CRC8_TABLE[crc ^ data[i]];
    }
    return crc ^ 0xFF;
}

// ---------- 编码器初始化 (含输出位宽配置与管道清空) ----------
void Encoder_Init(Encoder_t *encoder)
{
    // 1. 清空 SPI 管道
    KTM_SPI_Transmit32(KTM_CMD_READ_ANGLE, NULL);
    KTM_SPI_Transmit32(KTM_CMD_READ_ANGLE, NULL);

    // 2. 设置 SPI 角度输出位宽为 22 位，使总帧长为 32 位
    //    (22 位角度 + 2 位状态 + 8 位 CRC = 32 位)
    //    寄存器地址 S.RCBW 假设为 0x04 (需根据实际文档确认)
    //    0x00: 24位  0x01: 26位  0x02: 30位  0x03: 36位
    //    这里选择 22 位需查表，暂用 0x04 表示 22 位，无此选项则使用 26 位 + 6 位 CRC（需调整校验）
    //    稳妥做法：若芯片不支持 22 位，可跳过此设置，接收 36 位但舍弃低 4 位 CRC，此时 CRC 校验失效
    KTM_WriteReg(0x04, 0x0004);   // 示例：将角度宽度设为 22 位（请根据实际寄存器地址和值修改）

    // 3. 初始化软件变量
    encoder->angle = 0.0f;
    encoder->prev_angle = 0.0f;
    encoder->speed = 0.0f;
    encoder->angle_raw = 0.0f;
    encoder->encoder_value = 0;
    encoder->encoder_prev = 0;
    encoder->angle_offset = 0.0f;
    encoder->timestamp = 0;
    encoder->prev_timestamp = 0;

    // 4. 再清空一次管道，确保后续读取直接得到新配置下的数据
    KTM_SPI_Transmit32(KTM_CMD_READ_ANGLE, NULL);
    KTM_SPI_Transmit32(KTM_CMD_READ_ANGLE, NULL);
}

// ---------- 读取 KTM5910 磁编码器角度 (含 CRC 校验) ----------
void KTM5910_ReadAngle(Encoder_t *encoder)
{
    uint32_t rx_angle;
    uint32_t raw_angle;
    uint8_t crc_received, crc_calc;
    uint8_t data_for_crc[4];
    int retry = 0;

    do {
        // 重叠传输：第一次丢弃，第二次获取回复
        KTM_SPI_Transmit32(KTM_CMD_READ_ANGLE, NULL);
        KTM_SPI_Transmit32(KTM_CMD_READ_ANGLE, &rx_angle);

        // 提取字段：22 位角度[31:10], 2 位状态[9:8], 8 位 CRC[7:0]
        raw_angle   = (rx_angle >> 10) & 0x003FFFFF;   // 22 位
        uint8_t sta = (rx_angle >> 8)  & 0x03;
        crc_received = (uint8_t)(rx_angle & 0xFF);

        // 构造 CRC 校验的 32 位数据：高位补 4 个 0，然后 22 位角度 + 2 位状态
        data_for_crc[0] = (raw_angle >> 16) & 0xFF;    // 角度高 6 位 + 补 0 的高 2 位(实际补 0 在高位)
        data_for_crc[1] = (raw_angle >> 8)  & 0xFF;
        data_for_crc[2] = (raw_angle & 0xFF) | (sta << 6);  // 低 6 位角度 + 2 位状态在高位
        data_for_crc[3] = (sta & 0x03) << 6;             // 这里需要精确对齐，重做：
        /*
         * 更精确的方法：将 22 位角度 + 2 位状态 = 24 位，放在 32 位的高 24 位，
         * 低 8 位清零，然后计算 CRC。
         */
        uint32_t crc_word = (raw_angle << 8) | (sta << 6); // 22位角度左移8位 + 2位状态左移6位，共32位
        data_for_crc[0] = (crc_word >> 24) & 0xFF;
        data_for_crc[1] = (crc_word >> 16) & 0xFF;
        data_for_crc[2] = (crc_word >> 8)  & 0xFF;
        data_for_crc[3] = crc_word & 0xFF;

        crc_calc = KTM_CRC8(data_for_crc, 4);

        retry++;
    } while (crc_calc != crc_received && retry < 3);

    if (crc_calc != crc_received) {
        // CRC 校验连续失败，可选择保持上一次角度不变或报警
        // 这里简单返回，不更新角度
        return;
    }

    // 更新编码器数据
    encoder->encoder_value = raw_angle;
    encoder->angle_raw = (float)raw_angle * 360.0f / 4194304.0f;  // 2^22 = 4194304
    encoder->angle = encoder->angle_raw * DEG_TO_RAD + encoder->angle_offset;

    // 角度归一化到 [0, 2π)
    while (encoder->angle >= 2.0f * 3.141592653589793f)
        encoder->angle -= 2.0f * 3.141592653589793f;
    while (encoder->angle < 0.0f)
        encoder->angle += 2.0f * 3.141592653589793f;
}

// ---------- 以下函数保持不变 ----------
float Encoder_GetAngle(Encoder_t *encoder)
{
    return encoder->angle;
}

float Encoder_GetSpeed(Encoder_t *encoder, float dt)
{
    float angle_diff = encoder->angle - encoder->prev_angle;

    if (angle_diff > 3.141592653589793f)
        angle_diff -= 2.0f * 3.141592653589793f;
    else if (angle_diff < -3.141592653589793f)
        angle_diff += 2.0f * 3.141592653589793f;

    if (dt > 0.0f)
        encoder->speed = angle_diff / dt;
    else
        encoder->speed = 0.0f;

    return encoder->speed;
}

void Encoder_Update(Encoder_t *encoder, float dt)
{
    encoder->prev_angle = encoder->angle;
    KTM5910_ReadAngle(encoder);
    Encoder_GetSpeed(encoder, dt);
}

void Encoder_SetOffset(Encoder_t *encoder, float offset)
{
    encoder->angle_offset = offset;
}

void Encoder_Calibrate(Encoder_t *encoder)
{
    KTM5910_ReadAngle(encoder);
    encoder->angle_offset = -encoder->angle;
    KTM5910_ReadAngle(encoder);
}