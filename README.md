# STM32F407 无刷电机FOC驱动项目

## 项目简介

这是一个基于STM32F407的完整无刷电机驱动项目，支持FOC（磁场定向控制）和六步换相两种控制模式。

## 主要特性

- **FOC控制算法**：完整的Clark/Park变换、SVPWM生成
- **PID控制器**：双闭环控制（电流环+速度环）
- **多通信接口**：CAN总线、RS422、I2C
- **传感器支持**：霍尔传感器、KTM5910磁编码器
- **保护功能**：过流、过压、欠压、过温保护

## 硬件连接

### STM32F407与IR2103S连接
```
STM32F407        ->  IR2103S × 3
TIM1_CH1 (PA8)   ->  U相高侧PWM
TIM1_CH1N (PB13) ->  U相低侧PWM
TIM1_CH2 (PA9)   ->  V相高侧PWM
TIM1_CH2N (PB14) ->  V相低侧PWM
TIM1_CH3 (PA10)  ->  W相高侧PWM
TIM1_CH3N (PB15) ->  W相低侧PWM
```

### 霍尔传感器接口
```
霍尔A -> TIMx_CH1 (如PA0)
霍尔B -> TIMx_CH2 (如PA1)
霍尔C -> TIMx_CH3 (如PA2)
```

### KTM5910磁编码器 (SPI)
```
SPI_SCK  -> PA5
SPI_MISO -> PA6
SPI_MOSI -> PA7
SPI_CS   -> PA4
```

### 通信接口
```
CAN: PB8 (CAN_RX), PB9 (CAN_TX) + TJA1050
RS422: PA9 (TX), PA10 (RX) + MAX485
I2C: PB6 (SCL), PB7 (SDA)
```

## 快速开始

### 1. CubeMX配置
参考 `CubeMX_Configuration.md` 文件，按照步骤配置外设。

### 2. 生成代码
使用CubeMX生成Keil项目代码。

### 3. 添加用户代码
将 `User/` 文件夹下的所有文件添加到Keil项目中，并配置头文件路径。

### 4. 修改main.c
在CubeMX生成的 `main.c` 中添加：
```c
#include "main_control.h"

int main(void)
{
    // ... CubeMX生成的初始化代码 ...
    
    // 用户代码开始
    System_Init();
    System_Start();
    
    while (1)
    {
        Main_Loop();
    }
}
```

### 5. 编译下载
使用Keil编译并下载到STM32F407。

## 项目结构

```
├── Core/               # CubeMX生成的代码
├── Drivers/            # HAL库驱动
├── User/               # 用户代码
│   ├── Inc/            # 头文件
│   └── Src/            # 源文件
├── MDK-ARM/            # Keil项目文件
├── CubeMX_Configuration.md   # CubeMX配置说明
├── Keil_Configuration.md     # Keil配置说明
├── Project_Documentation.md  # 项目详细文档
└── README.md          # 本文件
```

## 控制命令示例

### CAN总线
- 设置速度：ID=0x100, 数据=浮点速度值(RPM)
- 设置电流：ID=0x101, 数据=浮点电流值(A)

### RS422 (自定义协议)
帧格式：`AA 55 CMD LEN DATA... CRC_LOW CRC_HIGH`
- 启动：AA 55 10 00 CRC
- 停止：AA 55 11 00 CRC
- 设置速度：AA 55 01 04 [float] CRC

## 参数调整

### PID参数
在 `User/Src/main_control.c` 中：
```c
PID_Init(&motor->speed_pid, 0.5f, 0.1f, 0.01f, 100.0f, 30.0f);
```

### FOC参数
在 `User/Src/foc.c` 中：
```c
foc->svpwm.Udc = 24.0f;           // 母线电压
foc->svpwm.max_modulation = 0.95f; // 最大调制比
```

## 注意事项

1. **电流采样**：需要根据实际硬件修改ADC通道和转换公式
2. **PID调参**：根据电机特性调整PID参数
3. **死区时间**：根据MOSFET和驱动芯片特性调整
4. **编码器校准**：首次使用需要运行校准程序

## 故障排查

参见 `Project_Documentation.md` 中的故障排查部分。

## 许可证

本项目仅供学习和研究使用。
