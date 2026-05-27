# Day 01: 项目启动与ESP-IDF开发环境搭建 | Project Launch & ESP-IDF Setup

> **今日目标:**
> - 了解ESP-SparkBot项目的整体架构
> - 安装VS Code和ESP-IDF开发插件
> - 搭建ESP-IDF v5.3/v5.4开发环境
> - 创建并编译第一个ESP-IDF项目: hello_world
> - 理解FreeRTOS任务的基本概念
>
> **产出:** 开发环境搭建完成，hello_world成功烧录并在串口输出

---

## 时间安排 | Schedule

| 时间 | 活动类型 | 内容 |
|------|----------|------|
| 09:00 - 09:15 | 晨间活动 | 项目介绍、学员互相认识、硬件套件发放 |
| 09:15 - 10:30 | 知识讲解 | ESP32-S3芯片介绍、ESP-IDF架构、FreeRTOS概念 |
| 10:30 - 10:45 | 课间休息 | |
| 10:45 - 12:00 | 动手实践 | 安装VS Code + ESP-IDF插件、创建hello_world项目 |
| 12:00 - 13:30 | 午餐休息 | |
| 13:30 - 15:00 | 项目实战 | 理解hello_world代码、修改代码、烧录运行 |
| 15:00 - 15:15 | 课间休息 | |
| 15:15 - 16:30 | 拓展练习 | 创建自定义任务、修改任务优先级 |
| 16:30 - 17:00 | 总结分享 | 环境搭建经验分享、常见问题总结 |

---

## 上午: ESP-IDF环境搭建 | Morning: ESP-IDF Setup

### 为什么要学这个? | Why Learn This?

ESP-IDF（Espressif IoT Development Framework）是乐鑫科技官方的开发框架，它提供了完整的工具链、API和示例代码，让你能用C语言开发ESP32系列芯片。这就像Python的`pip` + 标准库一样，是ESP32开发的基石。

ESP-IDF (Espressif IoT Development Framework) is the official development framework from Espressif. It provides a complete toolchain, API, and example code for developing ESP32 series chips in C. It is the foundation of ESP32 development, similar to pip + standard library in Python.

在嵌入式开发中，"环境搭建"往往是最容易劝退新人的一步。今天我们会一步步走完整个流程，确保每个人的电脑上都能编译和烧录代码。

In embedded development, "environment setup" is often the step that discourages beginners the most. Today we will go through the entire process step by step to ensure everyone can compile and flash code.

### 认识ESP32-S3 | Meet the ESP32-S3

```
ESP32-S3 芯片特性:
├── 双核 Xtensa LX7 处理器 @ 240MHz
├── 512KB SRAM + 8MB/16MB PSRAM
├── WiFi 2.4GHz (802.11 b/g/n)
├── Bluetooth 5.0 (BLE + Classic)
├── 45个可编程GPIO
├── 硬件加速: AES, SHA, RSA
├── AI加速: 向量指令集
└── 外设: I2C, SPI, I2S, UART, USB OTG, DVP摄像头接口
```

### 任务1.1: 安装VS Code和ESP-IDF插件 (20分钟)

**步骤:**

1. **安装VS Code**
   - 访问 https://code.visualstudio.com/
   - 下载Windows版本并安装
   - 推荐勾选："添加到PATH"、"添加到右键菜单"

2. **安装ESP-IDF扩展**
   - 打开VS Code
   - 点击左侧扩展图标（Ctrl+Shift+X）
   - 搜索 "ESP-IDF"
   - 安装 **Espressif IDF**（官方插件，注意选择Espressif发布的，不是第三方）
   - 安装后VS Code左侧会出现ESP-IDF图标

3. **配置ESP-IDF**
   - 点击左侧ESP-IDF图标
   - 选择 "Configure ESP-IDF Extension"
   - 选择 "Express" 安装模式
   - 选择 ESP-IDF 版本: **v5.3 或 v5.4**（推荐v5.4）
   - 选择安装路径: 建议使用默认路径（`C:\Users\你的用户名\esp\`）
   - 点击 "Install" 开始下载和安装

> **注意：** ESP-IDF完整安装需要下载约2-3GB文件，请确保有足够的磁盘空间（至少10GB）和稳定的网络连接。安装过程可能需要10-30分钟。

**预期结果:**
- VS Code左侧出现ESP-IDF图标（乐鑫Logo）
- 底部状态栏出现ESP-IDF相关按钮（编译、烧录、监视器等）
- 终端中执行 `idf.py --version` 输出版本号

**常见问题:**
- **下载速度慢**：ESP-IDF安装器默认从GitHub下载，国内可能较慢。可尝试使用[乐鑫官方国内镜像](https://dl.espressif.com/dl/esp-idf/)进行离线安装
- **Python版本冲突**：ESP-IDF需要Python 3.8+，如果电脑上已有多个Python版本，建议让ESP-IDF使用自带的Python环境
- **杀毒软件拦截**：部分杀毒软件可能误报ESP-IDF工具链，添加信任即可
- **PowerShell执行策略**：如果遇到脚本无法执行，运行 `Set-ExecutionPolicy -Scope CurrentUser RemoteSigned`

### 任务1.2: 创建第一个ESP-IDF项目 (30分钟)

**步骤:**

1. **使用VS Code创建项目**
   - 按 `Ctrl+Shift+P` 打开命令面板
   - 输入 "ESP-IDF: Show Examples"
   - 在示例列表中找到 "get-started/hello_world"
   - 点击 "Create project using example hello_world"
   - 选择一个存放路径（建议 `C:\Users\你的用户名\esp\projects\`）

2. **或者使用命令行创建**
   ```powershell
   # 在PowerShell中执行
   cd C:\Users\你的用户名\esp
   mkdir projects
   cd projects
   
   # 复制hello_world示例
   xcopy /E /I C:\Users\你的用户名\esp\esp-idf\examples\get-started\hello_world hello_world
   cd hello_world
   ```

3. **项目结构解析**
   ```
   hello_world/
   ├── CMakeLists.txt          # 顶层CMake构建配置
   ├── Makefile                # GNU Make构建（可选）
   ├── main/
   │   ├── CMakeLists.txt      # 主组件CMake配置
   │   ├── hello_world_main.c  # 主程序源代码
   │   └── idf_component.yml   # 组件依赖描述
   ├── sdkconfig               # 项目配置（由menuconfig生成）
   └── README.md
   ```

4. **阅读hello_world_main.c**
   ```c
   #include <stdio.h>
   #include "freertos/FreeRTOS.h"
   #include "freertos/task.h"
   #include "esp_system.h"
   #include "esp_spi_flash.h"

   void app_main(void)
   {
       printf("Hello world!\n");

       /* 打印芯片信息 */
       esp_chip_info_t chip_info;
       esp_chip_info(&chip_info);
       printf("This is %s chip with %d CPU core(s), WiFi%s%s, ",
               CONFIG_IDF_TARGET,
               chip_info.cores,
               (chip_info.features & CHIP_FEATURE_BTFDM) ? "/BT" : "",
               (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "/WiFi" : "");
       printf("silicon revision %d, ", chip_info.revision);
       printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
               (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
       printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

       /* 每1秒打印一次 */
       for (int i = 10; i >= 0; i--) {
           printf("Restarting in %d seconds...\n", i);
           vTaskDelay(1000 / portTICK_PERIOD_MS);
       }
       printf("Restarting now.\n");
       fflush(stdout);
       esp_restart();
   }
   ```

**代码解读:**
- `app_main()` 是ESP-IDF程序的入口函数（相当于普通C程序的`main()`，但它运行在一个FreeRTOS任务中）
- `printf()` 输出到串口（通过USB线可以在电脑上看到）
- `esp_chip_info()` 获取芯片信息 -- 这是一个典型的ESP-IDF API调用
- `vTaskDelay()` 是FreeRTOS的延时函数，参数是系统节拍数
- `portTICK_PERIOD_MS` 是每个系统节拍的毫秒数（默认10ms）
- `CONFIG_IDF_TARGET` 是在menuconfig中设置的目标芯片型号

### 任务1.3: 编译、烧录、监视 (30分钟)

**步骤:**

1. **设置目标芯片**
   ```powershell
   # 在项目目录中执行
   idf.py set-target esp32s3
   ```
   这个命令告诉ESP-IDF我们的目标是ESP32-S3芯片（而非ESP32或ESP32-C3等）。

2. **配置项目**（可选，先了解）
   ```powershell
   idf.py menuconfig
   ```
   这会打开一个文本菜单界面，可以配置各种选项。初学者暂时不需要修改，直接按Q退出即可。

3. **编译项目**
   ```powershell
   idf.py build
   ```
   首次编译需要约1-3分钟。成功后会在最后显示：
   ```
   Project build complete.
   ```

4. **连接开发板并烧录**
   - 用USB Type-C数据线连接ESP32-S3开发板到电脑
   - 检查串口：打开设备管理器 -> 端口（COM和LPT），看到类似 "COM3" 的串口
   - 烧录固件：
   ```powershell
   idf.py flash
   ```
   - 如果提示选择串口，使用 `idf.py -p COM3 flash`（替换COM3为你的实际串口号）

5. **监视串口输出**
   ```powershell
   idf.py monitor
   ```
   - 或者一步到位：`idf.py flash monitor`
   - 退出监视器：按 `Ctrl+]`

**预期结果:**
- 串口监视器中看到 "Hello world!" 输出
- 显示ESP32-S3的芯片信息（型号、核心数、Flash大小等）
- 倒数10秒后重启

**常见问题:**
- **"idf.py 不是内部命令"**：ESP-IDF环境变量未配置。运行 `C:\Users\你的用户名\esp\esp-idf\export.bat` 或重新打开VS Code
- **烧录失败 "Timed out waiting for packet header"**：按住开发板上的BOOT按钮，同时按一下RST按钮（或在烧录时按住BOOT），进入下载模式
- **串口找不到**：检查USB线是否支持数据传输（很多充电线只有供电功能），换个USB口试试
- **编译错误 "CONFIG_IDF_TARGET mismatch"**：运行 `idf.py fullclean` 然后 `idf.py set-target esp32s3`

---

## 下午: FreeRTOS任务基础 | Afternoon: FreeRTOS Task Basics

### 为什么要学FreeRTOS? | Why Learn FreeRTOS?

FreeRTOS是一个轻量级的实时操作系统（RTOS），它让ESP32能够"同时"运行多个任务。比如：一个任务读取传感器数据，一个任务刷新屏幕，一个任务处理网络请求——它们看起来是同时进行的。

FreeRTOS is a lightweight real-time operating system (RTOS) that allows the ESP32 to run multiple tasks "simultaneously". For example: one task reads sensor data, one refreshes the display, one handles network requests -- they appear to run at the same time.

**核心概念：**
- **任务（Task）**：一个独立运行的函数，有自己的栈空间
- **调度器（Scheduler）**：决定哪个任务在哪个时刻运行
- **时间片（Time Slice）**：每个任务能连续运行的最长时间
- **优先级（Priority）**：高优先级的任务会抢占低优先级的任务

```
时间线:
高优先级任务A:  ██░░░░░░░░████░░░░░░
低优先级任务B:  ░░████░░████░░████░░
空闲任务(idle): ░░░░░░██░░░░░░░░██░░

时间 →→→→→→→→→→→→→→→→→→→→→→→→→
```

### 任务1.4: 创建你自己的FreeRTOS任务 (40分钟)

**步骤:**

修改 `main/hello_world_main.c`，创建多个任务：

```c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

static const char *TAG = "main";  // 日志标签

// 任务1: LED闪烁模拟
void led_blink_task(void *pvParameters)
{
    int count = 0;
    while (1) {
        count++;
        ESP_LOGI(TAG, "LED Blink #%d - 亮!", count);
        vTaskDelay(500 / portTICK_PERIOD_MS);  // 亮500ms
        
        ESP_LOGI(TAG, "LED Blink #%d - 灭!", count);
        vTaskDelay(500 / portTICK_PERIOD_MS);  // 灭500ms
    }
}

// 任务2: 传感器读取模拟
void sensor_read_task(void *pvParameters)
{
    int temperature = 25;
    while (1) {
        temperature += (rand() % 3) - 1;  // 随机波动 +2到-2
        ESP_LOGI("sensor", "温度: %d°C", temperature);
        vTaskDelay(2000 / portTICK_PERIOD_MS);  // 每2秒读数一次
    }
}

// 任务3: 心跳任务
void heartbeat_task(void *pvParameters)
{
    while (1) {
        ESP_LOGI(TAG, "♥ 心跳 - 系统正常运行中");
        vTaskDelay(5000 / portTICK_PERIOD_MS);  // 每5秒心跳一次
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== ESP-SparkBot 启动 ===");
    
    // 打印芯片信息
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    ESP_LOGI(TAG, "芯片: %s, %d 核心, %dMB Flash",
             CONFIG_IDF_TARGET,
             chip_info.cores,
             spi_flash_get_chip_size() / (1024 * 1024));
    
    // 创建任务
    // 参数: 任务函数, 任务名, 栈大小, 参数, 优先级, 任务句柄, 核心
    xTaskCreate(led_blink_task,    // 任务函数
                "led_blink",       // 任务名称（用于调试）
                2048,              // 栈大小（字节）
                NULL,              // 参数（不需要传NULL）
                1,                 // 优先级（数字越大越高，0-24）
                NULL);             // 任务句柄（不需要获取就传NULL）
    
    xTaskCreate(sensor_read_task, "sensor_read", 2048, NULL, 1, NULL);
    xTaskCreate(heartbeat_task, "heartbeat", 2048, NULL, 1, NULL);
    
    ESP_LOGI(TAG, "所有任务已创建!");
    
    // app_main本身也是一个任务，创建完子任务后返回
    // 注意：app_main不能结束，否则系统会重启
    // 实际上app_main返回后系统会继续运行其他任务
}
```

**代码解读:**

| 元素 | 说明 |
|------|------|
| `ESP_LOGI(tag, format, ...)` | ESP-IDF的日志宏，比printf更好用，带时间戳和级别 |
| `xTaskCreate(...)` | 创建一个FreeRTOS任务 |
| `while(1)` | 任务的无尽循环 -- 嵌入式任务通常永远运行 |
| `vTaskDelay(ms / portTICK_PERIOD_MS)` | 让任务休眠指定毫秒数 |
| `2048` 栈大小 | 每个任务都有自己的栈，单位是字节 |
| 优先级 `1` | 相对于IDLE任务(0)，数字越大优先级越高 |

**预期结果:**
- 串口同时输出三个任务的信息
- LED闪烁任务每500ms打印一次
- 传感器任务每2秒打印一次温度
- 心跳任务每5秒打印一次

**常见问题:**
- **任务没运行**：检查栈大小是否足够，太小的栈可能导致栈溢出
- **输出混乱**：多个任务同时打印是正常的，ESP_LOGI内部有互斥保护
- **app_main返回后重启**：这是正常行为，实际项目中我们会创建永不结束的任务

### 任务1.5: 理解任务优先级 (20分钟)

修改代码，给三个任务不同的优先级，观察效果：

```c
// 修改任务创建时的优先级
xTaskCreate(led_blink_task, "led_blink", 2048, NULL, 3, NULL);      // 最高优先级
xTaskCreate(sensor_read_task, "sensor_read", 2048, NULL, 1, NULL); // 低优先级
xTaskCreate(heartbeat_task, "heartbeat", 2048, NULL, 2, NULL);     // 中等优先级
```

**实验:**
1. 把LED闪烁任务的优先级设为最高(3)，观察是否会影响其他任务的执行
2. 把所有任务设为相同优先级(1)，观察时间片轮转的效果
3. 试着把LED任务的`vTaskDelay`去掉（变成死循环），看高优先级任务是否会饿死低优先级任务

> **重要：** 在FreeRTOS中，任务必须主动放弃CPU（通过`vTaskDelay`或其他阻塞函数），否则同优先级或更低优先级的任务永远无法运行！

---

## 今日作业 | Homework

### 必做题
1. 完成ESP-IDF开发环境搭建，确保 `idf.py build` 能成功编译hello_world
2. 使用 `idf.py flash monitor` 烧录并观察串口输出
3. 修改hello_world代码，创建至少2个自定义任务，每个任务做不同的事情

### 挑战题
1. 创建一个任务，让它在启动5秒后删除自己（提示：使用 `vTaskDelete(NULL)`）
2. 使用 `uxTaskGetStackHighWaterMark(NULL)` 检查任务的栈使用情况
3. 尝试使用 `xTaskCreatePinnedToCore()` 将任务固定在指定CPU核心上运行

### 思考题
1. FreeRTOS的"实时"（Real-Time）是什么意思？为什么ESP32-S3需要运行操作系统？
2. 如果一个任务忘记调用`vTaskDelay()`会发生什么？这对其他任务有什么影响？
3. 栈（Stack）大小设置为2048字节意味着什么？如果设置太小会怎样？设置太大会怎样？

---

## 明日预告 | Tomorrow's Preview

明天我们将深入了解ESP32-S3的硬件架构，认识SparkBot的3块PCB板和各元器件的功能。同时开始焊接培训——你将学习如何安全地使用电烙铁，并亲手焊接你的第一块PCB！

Tomorrow we will dive into the hardware architecture of ESP32-S3, understand the 3 PCBs and components of SparkBot. We will also start soldering training -- you will learn to safely use a soldering iron and solder your first PCB!

---

## 参考资源 | References

- [ESP-IDF 编程指南 - 快速入门](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32s3/get-started/index.html)
- [ESP-IDF VS Code 插件指南](https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/tutorial/install.md)
- [FreeRTOS 入门教程（乐鑫官方）](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32s3/api-reference/system/freertos.html)
- [ESP32-S3 技术参考手册](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_cn.pdf)
- [ESP32入门 Arduino还是IDF? 一个视频给你讲清楚！](https://www.bilibili.com/video/BV1yCcnYfE7d/) -- B站，适合了解ESP-IDF与Arduino的区别
- [ESP32-IDF 开发环境搭建（B站搜索）](https://search.bilibili.com/all?keyword=ESP32-IDF%20%E5%BC%80%E5%8F%91%E7%8E%AF%E5%A2%83%E6%90%AD%E5%BB%BA)

*最后更新：2026-05-27*
