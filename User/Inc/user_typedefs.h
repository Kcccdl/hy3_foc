#ifndef __USER_TYPEDEFS_H
#define __USER_TYPEDEFS_H_

#include "stm32f4xx_hal.h"
#include "math.h"

// 常用数学常量
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#ifndef M_2PI
#define M_2PI 6.28318530717958647692f
#endif

// 角度转换宏
#define RAD_TO_DEG (180.0f / M_PI)
#define DEG_TO_RAD (M_PI / 180.0f)

// 限幅宏
#define LIMIT_MIN(x, min) ((x) < (min) ? (min) : (x))
#define LIMIT_MAX(x, max) ((x) > (max) ? (max) : (x))
#define LIMIT(x, min, max) (LIMIT_MIN(LIMIT_MAX(x, max), min))

// 绝对值宏
#define ABS(x) ((x) > 0 ? (x) : -(x))

// 符号函数
#define SIGN(x) ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))

// 交换宏
#define SWAP(x, y) do { typeof(x) temp = x; x = y; y = temp; } while(0)

// 数组长度宏
#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))

// 位操作宏
#define SET_BIT(reg, bit) ((reg) |= (1U << (bit)))
#define CLEAR_BIT(reg, bit) ((reg) &= ~(1U << (bit)))
#define TOGGLE_BIT(reg, bit) ((reg) ^= (1U << (bit)))
#define READ_BIT(reg, bit) (((reg) >> (bit)) & 1U)

// 错误代码定义
typedef enum
{
    ERR_NONE = 0x0000,
    ERR_OVER_CURRENT = 0x0001,
    ERR_UNDER_VOLTAGE = 0x0002,
    ERR_OVER_VOLTAGE = 0x0004,
    ERR_OVER_TEMP = 0x0008,
    ERR_HALL_ERROR = 0x0010,
    ERR_ENCODER_ERROR = 0x0020,
    ERR_CAN_ERROR = 0x0040,
    ERR_UART_ERROR = 0x0080,
} ErrorCode_t;

// 布尔类型定义 (如果标准库没有)
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// 浮点数比较宏 (考虑浮点误差)
#define FLOAT_EQ(a, b, eps) (fabsf((a) - (b)) < (eps))
#define FLOAT_ZERO(x) (fabsf(x) < 1e-6f)

// 时间相关宏
#define MS_TO_S(ms) ((float)(ms) / 1000.0f)
#define S_TO_MS(s) ((s) * 1000.0f)

// 转速转换
#define RPM_TO_RADS(rpm) ((rpm) * M_2PI / 60.0f)
#define RADS_TO_RPM(rads) ((rads) * 60.0f / M_2PI)

#endif /* __USER_TYPEDEFS_H_ */
