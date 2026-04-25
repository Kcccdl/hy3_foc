# STM32F407 无刷电机FOC驱动项目文档

## 项目概述

本项目是一个基于STM32F407的无刷电机驱动系统，支持FOC（磁场定向控制）和六步换相两种控制模式，具备完整的通信接口和监控功能。

## 硬件框架

### 主控芯片
- **STM32F407VGT6** (或其他F4系列)

### 功率驱动部分
- **IR2103S** × 3 (三相半桥驱动)
- 输出6路PWM: UH/UL, VH/VL, WH/WL

### 传感器接口
- **霍尔传感器** × 3 (HALL_A, HALL_B, HALL_C)
- **KTM5910磁编码器** (SPI接口)

### 通信接口
- **CAN总线** (TJA1050或其他CAN收发器)
- **RS422** (MAX485或专用RS422收发器)
- **I2C** (可接EEPROM、温度传感器等)

## 功能特性

1. **FOC控制算法**
   - Clark变换
   - Park变换
   - SVPWM生成
   - 电流环 + 速度环双闭环控制

2. **PID控制器**
   - 位置式PID
   - 增量式PID
   - 积分限幅、输出限幅

3. **通信协议**
   - CANopen协议或自定义CAN协议
   - RS422自定义帧协议 (带CRC16校验)
   - I2C设备读写

4. **保护功能**
   - 过流保护
   - 过压/欠压保护
   - 过温保护
   - 故障自动停机

## 文件结构

```
BLDC_FOC_Driver/
├── Core/
│   ├── Inc/
│   │   └── main.h
│   └── Src/
│       └── main.c          (CubeMX生成，需添加用户代码)
├── Drivers/
│   ├── CMSIS/
│   └── STM32F4xx_HAL_Driver/
├── User/
│   ├── Inc/
│   │   ├── foc.h            (FOC控制头文件)
│   │   ├── pid.h            (PID控制器头文件)
│   │   ├── bldc.h           (BLDC电机驱动头文件)
│   │   ├── encoder.h        (磁编码器头文件)
│   │   ├── can_comm.h       (CAN通信头文件)
│   │   ├── rs422_comm.h     (RS422通信头文件)
│   │   ├── i2c_dev.h        (I2C设备头文件)
│   │   └── main_control.h   (主控制逻辑头文件)
│   └── Src/
│       ├── foc.c            (FOC控制实现)
│       ├── pid.c            (PID控制器实现)
│       ├── bldc.c           (BLDC电机驱动实现)
│       ├── encoder.c        (磁编码器实现)
│       ├── can_comm.c       (CAN通信实现)
│       ├── rs422_comm.c     (RS422通信实现)
│       ├── i2c_dev.c        (I2C设备实现)
│       ├── main_control.c   (主控制逻辑实现)
│       └── main_user.c      (用户主函数示例)
├── MDK-ARM/
│   └── BLDC_FOC_Driver.uvprojx
├── CubeMX_Configuration.md  (CubeMX配置说明)
├── Keil_Configuration.md    (Keil配置说明)
└── Project_Documentation.md (本文件)
```

## 快速开始

### 1. CubeMX配置
参考 `CubeMX_Configuration.md` 完成以下配置：
- 时钟树配置 (168MHz)
- TIM1 PWM输出 (6路，带死区)
- TIM2/TIM3/TIM4 霍尔接口
- SPI1 磁编码器接口
- CAN1 通信接口
- USART2 RS422接口
- I2C1 通用接口
- ADC1 电流采样
- TIM6 系统节拍定时器

### 2. 生成代码
在CubeMX中点击右上角的 "GENERATE CODE" 按钮（全大写），在Project Manager中已选择 "MDK-ARM" 工具链。

### 3. Keil项目配置
参考 `Keil_Configuration.md` 配置Keil项目：
- 添加User文件夹到项目
- 配置头文件路径
- 设置优化等级

### 4. 编译下载
- 编译项目 (F7)
- 连接ST-Link调试器
- 下载程序 (F8)

## 控制命令示例

### CAN总线命令
- 设置速度: ID=0x100, Data=[float speed_rpm]
- 设置电流: ID=0x101, Data=[float current_a]
- 读取状态: 接收ID=0x200的反馈

### RS422命令 (自定义协议)
帧格式: `AA 55 CMD LEN DATA... CRC_LOW CRC_HIGH`

示例命令:
- 设置速度: `AA 55 01 04 [float] CRC`
- 启动电机: `AA 55 10 00 CRC`
- 停止电机: `AA 55 11 00 CRC`

## 参数调整

### PID参数
在 `main_control.c` 的 `System_Init()` 函数中调整：
```c
// 速度PID
PID_Init(&motor->speed_pid, 0.5f, 0.1f, 0.01f, 100.0f, 30.0f);
// 电流PID
PID_Init(&motor->current_pid, 2.0f, 0.5f, 0.0f, 50.0f, 24.0f);
```

### FOC参数
在 `foc.c` 的 `FOC_Init()` 函数中调整：
```c
foc->svpwm.Udc = 24.0f;           // 母线电压
foc->svpwm.max_modulation = 0.95f; // 最大调制比
```

## 注意事项

1. **电流采样**: 需要根据实际硬件修改ADC采样和转换公式
2. **PWM死区时间**: 根据IR2103S和MOSFET特性调整死区时间
3. **编码器校准**: 首次使用需要运行编码器校准程序
4. **PID调参**: 根据实际电机特性调整PID参数
5. **保护功能**: 确保过流、过压等保护电路正常工作

## 故障排查

### 电机不转
1. 检查PWM输出是否正常
2. 检查霍尔信号是否正确
3. 检查PID参数是否合理
4. 查看系统状态 `sys_state`

### 通信异常
1. 检查CAN/RS422收发器连接
2. 检查波特率设置
3. 使用逻辑分析仪查看波形

### 过流保护
1. 检查电流采样电路
2. 降低PID输出限幅
3. 检查电机相序

## 更新日志

- 2024-01-01: 初始版本
  - 实现FOC控制算法
  - 实现PID控制器
  - 实现CAN/RS422通信
  - 实现保护功能
