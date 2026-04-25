# STM32F407 无刷电机驱动 CubeMX 配置指南

## 1. 新建项目

1. 打开 STM32CubeMX
2. 点击 "New Project"
3. 在搜索框输入 "STM32F407VG"（或你使用的具体型号，如 STM32F407ZE、STM32F407IG 等）
4. 双击选中的芯片，进入Pinout视图

## 2. 系统配置

### 2.1 时钟源配置
- 在左侧边栏 "System Core" -> 点击 "RCC"：
  - High Speed Clock (HSE): 点击下拉框选择 "Crystal/Ceramic Resonator"
  - Low Speed Clock (LSE): 点击下拉框选择 "Disable"（除非需要RTC）
- 在左侧边栏 "System Core" -> 点击 "SYS"：
  - Debug: 点击下拉框选择 "Serial Wire"（用于ST-Link调试）

### 2.2 时钟树配置 (Clock Configuration)
点击顶部 "Clock Configuration" 标签页：
1. 在 "Input frequency" 输入框输入 8（外部晶振8MHz）
2. 点击 "PLL Source Mux" 选择 HSE
3. 在 "PLLM" 输入框输入 8（8MHz / 8 = 1MHz）
4. 在 "PLLN" 输入框输入 336（1MHz × 336 = 336MHz）
5. 在 "PLLP" 下拉框选择 2（336MHz / 2 = 168MHz）
6. 点击 "System Clock Mux" 选择 PLLCLK
7. 在 "AHB Prescaler" 下拉框选择 1（168MHz）
8. 在 "APB1 Prescaler" 下拉框选择 4（42MHz）
9. 在 "APB2 Prescaler" 下拉框选择 2（84MHz）

验证：右侧 "HCLK (MHz)" 应显示 168 MHz

## 3. PWM输出配置 (TIM1 高级定时器)

### 3.1 选择定时器
- 在左侧边栏 "Timers" -> 点击 "TIM1"：
  - 勾选 "Internal Clock"
  - 勾选 "Channel1" -> 下拉选择 "PWM Generation CH1"
  - 勾选 "Channel2" -> 下拉选择 "PWM Generation CH2"
  - 勾选 "Channel3" -> 下拉选择 "PWM Generation CH3"
  - 勾选 "Channel1N" -> 下拉选择 "PWM Generation CH1N"（互补输出）
  - 勾选 "Channel2N" -> 下拉选择 "PWM Generation CH2N"
  - 勾选 "Channel3N" -> 下拉选择 "PWM Generation CH3N"

### 3.2 参数配置
点击 "Parameter Settings" 标签页：
- Prescaler (PSC): 0
- Counter Mode: Up
- Counter Period (ARR): 4200 - 1（84MHz/4200 = 20kHz PWM）
- Internal Clock Division: No Division
- Repetition Counter: 0
- Auto-Reload Preload: Enable

### 3.3 死区时间配置
点击 "Parameter Settings" 标签页，找到 "Dead Time" 区域：
- Dead Time: 100（约200ns，根据MOSFET和驱动芯片调整，单位：DCLK周期）

### 3.4 中断配置
点击 "NVIC Settings" 标签页：
- TIM1 Update Interrupt: 勾选 Enable（用于PWM更新中断，可选）

### 3.5 引脚分配
TIM1默认引脚：
- TIM1_CH1 (PA8) -> IR2103S U相高侧
- TIM1_CH1N (PB13) -> IR2103S U相低侧
- TIM1_CH2 (PA9) -> IR2103S V相高侧
- TIM1_CH2N (PB14) -> IR2103S V相低侧
- TIM1_CH3 (PA10) -> IR2103S W相高侧
- TIM1_CH3N (PB15) -> IR2103S W相低侧

## 4. 霍尔传感器接口配置

### 4.1 选择定时器
- 在左侧边栏 "Timers" -> 点击 "TIM2"（或TIM3/TIM4）：
  - Combined Channels: 下拉选择 "Hall Sensor"
  - 此时会自动配置三个通道为输入捕获模式

### 4.2 参数配置
- Prescaler: 0
- Counter Period: 0xFFFF（最大计数值）
- Counter Mode: Up
- Auto-Reload Preload: Disable

### 4.3 中断配置
- TIM2 Global Interrupt: 勾选 Enable
- 优先级设为 1（较高优先级，用于换相）

### 4.4 引脚分配
假设使用TIM2：
- TIM2_CH1 (PA0) -> 霍尔传感器 A
- TIM2_CH2 (PA1) -> 霍尔传感器 B
- TIM2_CH3 (PA2) -> 霍尔传感器 C

在GPIO模式下，这三个引脚应设为 "Pull-up"（上拉输入）

## 5. SPI配置 (KTM5910磁编码器)

### 5.1 选择SPI
- 在左侧边栏 "Connectivity" -> 点击 "SPI1"：
  - Mode: 下拉选择 "Full-Duplex Master"
  - Hardware NSS Signal: 下拉选择 "Disable"（软件控制CS）

### 5.2 参数配置
点击 "Parameter Settings" 标签页：
- Frame Format: Motorola
- Data Size: 8 bits（或16 bits，根据KTM5910规格）
- Prescaler: 16（84MHz/16 = 5.25MHz）
- Clock Polarity: Low
- Clock Phase: 1 Edge（或根据KTM5910 datasheet调整）
- CRC Calculation: Disable
- NSS Signal Type: Software

### 5.3 引脚分配
SPI1默认引脚：
- SPI1_SCK (PA5)
- SPI1_MISO (PA6)
- SPI1_MOSI (PA7)
- 额外配置一个GPIO作为CS片选：PA4（在右侧芯片视图中点击PA4 -> 选择 "GPIO_Output"）

## 6. CAN配置

### 6.1 选择CAN
- 在左侧边栏 "Connectivity" -> 点击 "CAN1"：
  - 勾选 "Activated"

### 6.2 参数配置
点击 "Parameter Settings" 标签页：

**方法一：直接输入目标波特率（最简单、最准确，强烈推荐）**
- 在 "Baud Rate" 输入框中直接输入 `500000`（表示500 kbps）
- 或输入 `1000000`（表示1 Mbps）
- CubeMX 会自动计算并填充下面的参数
- **重要**：观察顶部显示的 "Baud Rate" 数值，确认是你要的波特率

**方法二：点击 "..." 按钮自动计算**
- 点击 "Baud Rate" 右侧的 "..." 按钮
- 在弹出窗口中输入目标波特率（如 500000）
- CubeMX 会自动计算并填充所有参数
- 点击 OK 确认

**手动配置（不推荐，容易出错）**
- 在 "Bit Timing Parameters" 区域手动设置：
  - **Prescaler (BRP)**: 6
  - **Time Quantum in Bit Segment 1 (BS1)**: 10
  - **Time Quantum in Bit Segment 2 (BS2)**: 3
  - **ReSync Jump Width (SJW)**: 1
- 验证：顶部 "Baud Rate" 应显示 `500000 bps`
- 计算公式（CAN时钟=42MHz）：`42MHz / (6 × (1 + 10 + 3)) = 42MHz / 84 = 500kbps`

**1Mbps手动配置（CAN时钟=42MHz）**
- **Prescaler (BRP)**: 3
- **Time Quantum in Bit Segment 1 (BS1)**: 10
- **Time Quantum in Bit Segment 2 (BS2)**: 3
- **ReSync Jump Width (SJW)**: 1
- 验证：`42MHz / (3 × 14) = 1Mbps`

**模式设置**
- 无需手动设置模式，CubeMX 默认为 **Normal** 模式（非环回模式）
- 如需环回测试，可在代码初始化后添加：`hcan1.Init.Mode = CAN_MODE_LOOPBACK;`

### 6.3 中断配置
- CAN1 RX0 Interrupt: 勾选 Enable
- 优先级设为 2

### 6.4 引脚分配
CAN1默认引脚：
- CAN1_RX (PB8) -> 配置为 AF9_CAN1
- CAN1_TX (PB9) -> 配置为 AF9_CAN1

## 7. USART配置 (RS422)

### 7.1 选择USART
- 在左侧边栏 "Connectivity" -> 点击 "USART2"：
  - Mode: 下拉选择 "Asynchronous"

**注意：USART2默认引脚冲突！**
- USART2_TX (PA2) 与 TIM2_CH3 (PA2) 冲突（TIM2用于霍尔传感器）
- **解决方案（三选一）**：

**方案A：USART2重映射到备用引脚（推荐）**
- 在芯片视图中，找到 PD5 引脚，点击选择 "USART2_TX"
- 找到 PD6 引脚，点击选择 "USART2_RX"
- 这样 USART2 使用 PD5/PD6，不与 TIM2 冲突

**方案B：改用USART3（推荐）**
- 在左侧边栏 "Connectivity" -> 点击 "USART3"：
  - Mode: 下拉选择 "Asynchronous"
- USART3默认引脚：
  - USART3_TX (PB10)
  - USART3_RX (PB11)
- 注意：代码中需将 `huart2` 改为 `huart3`

**方案C：霍尔传感器改用TIM3**
- TIM3默认引脚：
  - TIM3_CH1 (PA6) -> 霍尔A
  - TIM3_CH2 (PA7) -> 霍尔B
  - TIM3_CH3 (PB0) -> 霍尔C
- 注意：代码中需将 `htim2` 改为 `htim3`

### 7.2 参数配置
- Baud Rate: 115200（或921600等）
- Word Length: 8 bits
- Parity: None
- Stop Bits: 1
- Over Sampling: 16 Times

### 7.3 中断配置
- 根据选择的USART，勾选对应的Global Interrupt：
  - USART2：USART2 Global Interrupt
  - USART3：USART3 Global Interrupt

### 7.4 引脚分配
**使用USART2（重映射后）**：
- USART2_TX (PD5)
- USART2_RX (PD6)

**使用USART3（推荐）**：
- USART3_TX (PB10)
- USART3_RX (PB11)

## 8. I2C配置

### 8.1 选择I2C
- 在左侧边栏 "Connectivity" -> 点击 "I2C1"：
  - Mode: 下拉选择 "I2C"

### 8.2 参数配置
- I2C Speed Mode: Fast Mode (400 kHz)
- 其他参数保持默认

### 8.3 引脚分配
I2C1默认引脚：
- I2C1_SCL (PB6)
- I2C1_SDA (PB7)

## 9. ADC配置 (电流采样)

### 9.1 选择ADC
- 在左侧边栏 "Analog" -> 点击 "ADC1"：
  - **重要：避免与霍尔传感器引脚冲突！**
  - 霍尔传感器使用：PA0(霍尔A)、PA1(霍尔B)、PA2(霍尔C)
  - **解决方案（三选一）**：

**方案A：ADC改用PC0/PC1/PC2（推荐）**
- 勾选 "IN10"（PC0 -> 电流A）
- 勾选 "IN11"（PC1 -> 电流B）
- 勾选 "IN12"（PC2 -> 电流C）
- 修改代码中ADC通道对应关系

**方案B：霍尔传感器改用TIM3**
- 在左侧边栏 "Timers" -> 点击 "TIM3"：
  - Combined Channels: 选择 "Hall Sensor"
- TIM3默认引脚：
  - TIM3_CH1 (PA6) -> 霍尔A
  - TIM3_CH2 (PA7) -> 霍尔B
  - TIM3_CH3 (PB0) -> 霍尔C
- ADC仍可使用PA0/PA1/PA2（IN0/IN1/IN2）
- 代码中需将 `htim2` 改为 `htim3`

**方案C：使用ADC2/ADC3**
- 在左侧边栏 "Analog" -> 点击 "ADC2"：
  - 勾选 "IN0"、"IN1"、"IN2"
- 释放ADC1给其他用途

### 9.2 参数配置（以方案A为例，ADC1 + PC0/PC1/PC2）
- Mode: Independent Mode
- Data Alignment: Right Alignment
- Scan Conversion Mode: Enable
- Continuous Conversion Mode: Disable（使用触发模式）
- Discontinuous Conversion Mode: Disable
- DMA Continuous Requests: Enable（使用DMA）
- End of Conversion Selection: EOC flag at the end of all conversions

### 9.3 添加DMA
点击 "DMA Settings" 标签页：
- 点击 "Add" 按钮添加 DMA Request: ADC1
- Channel: DMA2 Stream0（或自动分配）
- Direction: Peripheral To Memory
- Priority: High

### 9.4 代码修改（若选择方案A）
在 `User/Src/current_sense.c` 中：
- 修改ADC通道：IN10(PC0), IN11(PC1), IN12(PC2)
- 或保持IN0/IN1/IN2，但硬件连接改为PC0/PC1/PC2

## 10. 定时器配置 (系统节拍)

### 10.1 选择TIM6
- 在左侧边栏 "Timers" -> 点击 "TIM6"：
  - **注意**：TIM6是基本定时器，没有"Internal Clock"勾选项，默认使用内部时钟
  - 直接进行参数配置即可

### 10.2 参数配置
点击 "Parameter Settings" 标签页：
- Prescaler: 8400 - 1（84MHz/8400 = 10kHz）
- Counter Period: 10 - 1（10kHz/10 = 1kHz，即1ms中断一次）
- Auto-Reload Preload: Enable

### 10.3 中断配置
点击 "NVIC Settings" 标签页：
- TIM6 Global Interrupt: 勾选 Enable

## 11. GPIO配置

### 11.1 状态指示LED
- 选择一个GPIO引脚（如PB0）设为 "GPIO_Output"
- 在 "User Label" 中输入 "LED_STATUS"

### 11.2 电机使能引脚（如果需要）
- 选择GPIO引脚（如PB1）设为 "GPIO_Output"
- 在 "User Label" 中输入 "MOTOR_EN"

### 11.3 故障输入引脚
- 选择GPIO引脚（如PB2）设为 "GPIO_Input"
- 在 "User Label" 中输入 "FAULT"

## 12. NVIC配置

点击 "System Core" -> 点击 "NVIC"：
- TIM2 Global Interrupt: Priority 1, Enable
- CAN1 RX0 Interrupt: Priority 2, Enable
- USART2 Interrupt: Priority 2, Enable
- TIM6 Global Interrupt: Priority 3, Enable
- ADC_IRQn: Priority 2, Enable

## 13. 生成代码

1. 点击顶部 "Project Manager" 标签页
2. Project Name: 输入 "BLDC_FOC_Driver"
3. Project Location: 点击 "Browse" 选择保存路径
4. Toolchain / IDE: 下拉选择 "MDK-ARM"（Keil）
5. 点击 "Code Generator" 标签页：
   - 勾选 "Generate peripheral initialization as a pair of .c/.h files per peripheral"
   - 勾选 "Keep User Code when re-generating"
   - 勾选 "Delete previously generated files when not re-generated"
6. 点击右上角全大写 "GENERATE CODE" 按钮
7. 生成完成后，点击 "Open Project" 打开Keil项目

## 注意事项

1. **引脚冲突**: 配置时注意检查引脚是否有冲突（CubeMX会用红色提示）
2. **时钟配置**: 确保系统时钟达到168MHz，这是F407的最高性能
3. **PWM频率**: 20kHz是比较常用的PWM频率，可根据需要调整ARR值
4. **死区时间**: 200ns是典型值，需根据MOSFET开关特性调整
5. **ADC采样**: 电流采样需要精确同步，建议使用定时器触发+DMA方式
6. **CAN波特率**: 最可靠的方法是直接在 "Baud Rate" 输入框输入目标值（如500000），让CubeMX自动计算
