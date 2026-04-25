# Keil MDK-ARM 配置指南

## 1. 打开项目

1. 使用CubeMX生成代码后，点击 "Open Project" 打开Keil
2. 或者手动打开：`BLDC_FOC_Driver\MDK-ARM\BLDC_FOC_Driver.uvprojx`

## 2. 项目结构检查

打开项目后，在Project窗口应看到如下结构：
```
Project Target 1
├── Core
│   ├── Inc
│   │   ├── main.h
│   │   ├── stm32f4xx_it.h
│   │   └── ...
│   └── Src
│       ├── main.c
│       ├── stm32f4xx_it.c
│       └── ...
├── Drivers
│   ├── CMSIS
│   └── STM32F4xx_HAL_Driver
└── MDK-ARM
    └── startup_stm32f407xx.s
```

## 3. 添加用户代码文件夹

### 3.1 创建User目录
在项目中创建 `User` 文件夹：
1. 在项目根目录 `BLDC_FOC_Driver` 下新建文件夹 `User`
2. 在 `User` 下创建 `Inc` 和 `Src` 两个子文件夹

### 3.2 将用户代码文件添加到项目
1. 在Keil Project窗口中，右键点击 "Target 1"（或项目名称）
2. 选择 "Add Group..."
3. 输入组名 "User" 并确认
4. 右键点击新建的 "User" 组
5. 选择 "Add Existing Files to Group..."
6. 浏览到 `User\Src` 目录，文件类型选择 "C Files (*.c)"，选择所有 `.c` 文件后点击 "Add"
   - foc.c
   - pid.c
   - bldc.c
   - encoder.c
   - can_comm.c
   - rs422_comm.c
   - i2c_dev.c
   - ir2103s_driver.c
   - current_sense.c
   - main_control.c
   - main_user.c

## 4. 配置头文件路径

1. 点击工具栏的 "Options for Target" 按钮（或按Alt+F7）
2. 选择 "C/C++" 标签页
3. 在 "Include Paths" 框中，点击右侧的 "..." 按钮
4. 添加以下路径（根据实际位置调整）：
   - `.\User\Inc`
   - `.\Core\Inc`
   - `.\Drivers\CMSIS\Include`
   - `.\Drivers\STM32F4xx_HAL_Driver\Inc`
5. 点击 "OK" 保存

## 5. C/C++ 配置

在 "Options for Target" -> "C/C++" 标签页：

### 5.1 预处理定义
在 "Define" 框中，确保包含：
```
USE_HAL_LIBRARY,STM32F407xx
```

### 5.2 优化等级
- Optimization: 选择 "Level 0 (-O0)"（调试阶段）
  - 或选择 "Level 1 (-O1)"（发布阶段，优化速度）
- 勾选 "One ELF Section per Function"（优化代码大小）

### 5.3 其他设置
- Warnings: 下拉框选择 "All Warnings"（显示所有警告）
- 勾选 "C99" 或 "C99 Mode"（使用C99标准，不同Keil版本名称可能不同）

## 6. Debug 配置

### 6.1 选择调试器
1. 点击 "Options for Target" -> "Debug" 标签页
2. Select: 选择 "ST-Link Debugger"
3. 点击右侧的 "Settings" 按钮

### 6.2 Debug设置
在 "ST-Link Debugger Settings" 窗口：
1. Debug: 选择 "SW"（Serial Wire调试接口）
2. Clock: 选择 "4MHz"（或自适应）
3. 勾选 "Verify Code Download"
4. 勾选 "Download to Flash"

### 6.3 Flash Download设置
在 "Settings" 窗口中点击 "Flash Download" 标签页：
1. 勾选 "Reset and Run"（下载后自动运行）
2. 点击 "Add" 按钮，选择正确的Flash算法：
   - 对于STM32F407VG: 选择 "STM32F4xx 1MB Flash"
   - 对于其他容量芯片，选择对应的Flash算法
3. 在 "Erase" 区域选择 "Erase Full Chip" 或 "Erase Sectors"
4. 点击 "OK" 保存设置

## 7. Utilities 配置

1. 点击 "Options for Target" -> "Utilities" 标签页
2. 选择 "Use Debug Driver"（使用调试驱动下载）
3. 注："Update Target before Debugging" 选项在某些Keil版本中可能位于 "Debug" 标签页的Settings中

## 8. 编译项目

### 8.1 第一次编译
1. 点击工具栏的 "Build" 按钮（或按F7）
2. 查看Build Output窗口的编译信息
3. 如有错误，根据提示修改代码

### 8.2 常见编译错误及解决

**错误1: 找不到头文件**
```
error: xxx.h: No such file or directory
```
解决：检查 "Include Paths" 是否配置正确

**错误2: 未定义的引用**
```
error: undefined reference to `XXX'
```
解决：检查对应的.c文件是否已添加到项目中

**错误3: 重复定义**
```
error: multiple definition of `XXX'
```
解决：检查是否有两个文件定义了同名函数

## 9. 下载程序

### 9.1 连接硬件
1. 使用ST-Link V2调试器
2. 连接：SWDIO、SWCLK、GND、3.3V（可选）
3. 给目标板供电

### 9.2 下载
1. 点击工具栏的 "Load" 按钮（或按F8）
2. 等待下载完成
3. 程序会自动运行（如果勾选了Reset and Run）

## 10. 调试配置

### 10.1 开始调试
1. 点击工具栏的 "Start/Stop Debug Session" 按钮（或按Ctrl+F5）
2. 程序会停在main函数入口

### 10.2 常用调试功能
- **单步进入**: F11 (Step Into)
- **单步跳过**: F10 (Step Over)  
- **跳出函数**: Ctrl+F11 (Step Out)
- **运行**: F5 (Run)
- **暂停**: F5 (Pause，与Run同一按钮切换)
- **设置/取消断点**: 点击代码行左侧或按F9
- **查看变量**: 在代码中选中变量，右键选择 "Add to Watch 1" 或 "Quick Watch"

### 10.3 查看外设寄存器
1. 在调试模式下，点击菜单 "Peripherals"
2. 选择要查看的外设（如TIM1、ADC等）
3. 会弹出寄存器窗口，可实时查看寄存器值

## 11. 使用printf输出（可选）

### 11.1 配置MicroLIB
1. "Options for Target" -> "Target" 标签页
2. 勾选 "Use MicroLIB"

### 11.2 重定向printf
在 `main.c` 或新建的 `retarget.c` 中添加：
```c
#include <stdio.h>

int fputc(int ch, FILE *f)
{
    // 使用USART2输出（根据配置修改）
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
```

### 11.3 使用printf
```c
printf("Motor Speed: %.2f RPM\r\n", speed);
```

## 12. 代码大小优化

编译后，Build Output窗口会显示代码大小：
```
Program Size: Code=xxxx RO-data=xxxx RW-data=xxxx ZI-data=xxxx
```

- Code: 代码区（Flash）
- RO-data: 只读数据（Flash）
- RW-data: 可读写数据（Flash + RAM）
- ZI-data: 零初始化数据（RAM）

如需减小代码大小：
1. 提高优化等级到 -O2 或 -O3
2. 使用 "One ELF Section per Function"
3. 删除不必要的库和代码

## 13. 版本控制集成（可选）

### 13.1 忽略文件
创建 `.gitignore` 文件，内容：
```
MDK-ARM/*.axf
MDK-ARM/*.o
MDK-ARM/*.d
MDK-ARM/*.crf
MDK-ARM/*.htm
MDK-ARM/*.lnp
MDK-ARM/*.sct
MDK-ARM/*.dep
MDK-ARM/RTE/
Debug/
Release/
```

## 注意事项

1. **中断优先级**: 确保中断优先级配置合理，避免死锁
2. **栈大小**: 如程序复杂，可能需要增大栈大小（在startup文件中修改）
3. **Flash保护**: 如果芯片有读保护，需要先解除保护才能下载
4. **时钟配置**: 确保外部晶振频率与硬件一致
5. **复位电路**: 确保复位电路正常工作

## 故障排查

### 下载失败
- 检查ST-Link驱动是否安装
- 检查接线是否正确
- 尝试降低SWD频率
- 检查芯片是否上电

### 程序不运行
- 检查复位电路
- 检查时钟配置
- 查看是否进入HardFault
- 使用调试器单步执行查找问题

### PWM无输出
- 检查TIM1时钟是否使能
- 检查GPIO配置是否正确
- 检查PWM是否启动（HAL_TIM_PWM_Start）
