# AICat 故障排除指南 / AICat Troubleshooting Guide

> Advanced Track - Desktop Pet Hardware
> 高级赛道 - 桌面宠物硬件

---

## 快速诊断流程 / Quick Diagnosis Flowchart

```
    问题发生 / Problem Occurs
         │
         ▼
    ┌─────────────────┐
    │ ESP32 重启/死机? │── Yes ──→ 见第 1 节: 电源问题
    └────────┬────────┘
             │ No
             ▼
    ┌─────────────────┐
    │ 上传代码失败?    │── Yes ──→ 见第 8 节: 上传问题
    └────────┬────────┘
             │ No
             ▼
    ┌─────────────────┐
    │ 屏幕不显示?      │── Yes ──→ 见第 5 节: 屏幕问题
    └────────┬────────┘
             │ No
             ▼
    ┌─────────────────┐
    │ 舵机不动?        │── Yes ──→ PCA9685? → 见第 7 节
    └────────┬────────┘           STS3032?  → 见第 4 节
             │ No
             ▼
    ┌─────────────────┐
    │ 没有声音?        │── Yes ──→ 见第 3 节: 音频问题
    └────────┬────────┘
             │ No
             ▼
    ┌─────────────────┐
    │ 摄像头不工作?    │── Yes ──→ 见第 2 节: 摄像头问题
    └────────┬────────┘
             │ No
             ▼
    ┌─────────────────┐
    │ WiFi 连不上?     │── Yes ──→ 见第 6 节: 网络问题
    └────────┬────────┘
             │ No
             ▼
    检查串口输出中的错误信息
    Check serial output for error messages
```

---

## 1. ESP32 舵机运动时重启 / ESP32 Resets When Servos Move

这是最常见的问题, 几乎总是电源相关.

This is the most common problem, almost always power-related.

### 症状 / Symptoms

- 舵机一动作, ESP32 就自动重启
- 串口输出 "Guru Meditation Error" 或乱码
- 系统间歇性死机
- LED 闪烁后熄灭

### 原因分析 / Root Cause

```
    舵机启动瞬间电流可达 500mA-1A (每个)
    4 个总线舵机同时启动 = 2A-4A 瞬态电流!

    如果舵机从 XIAO 5V 引脚取电:
    ┌──────────┐     USB 500mA     ┌──────────┐
    │  电脑 USB │ ──── (上限) ──── │   XIAO   │
    └──────────┘                   │  5V 引脚 │──→ 舵机 (需要 2A+)
                                   └──────────┘
                                   ↑ 电流不足 → 电压骤降 → ESP32 重启!
```

### 解决方案 / Solutions

#### 方案 A: 独立外部电源 (推荐) / Separate External Power (Recommended)

```
    ┌──────────┐     USB 500mA     ┌──────────┐
    │  电脑 USB │ ──────────────── │   XIAO   │
    └──────────┘                   │  (逻辑)   │
                                   └─────┬────┘
                                         │ GND 相连
    ┌──────────────┐                     │
    │ 外部 5V/6V   │──→ 舵机电源 (V+)    │
    │ >= 3A 输出   │──→ GND (共地) ──────┘
    └──────────────┘
```

- 使用 5V 3A 以上的电源模块或适配器
- 外部电源 GND 必须与 XIAO GND 连接 (公共地)
- 推荐电源: 5V 3A DC-DC 降压模块, 或 USB 5V 2A+ 充电头

#### 方案 B: 增加滤波电容 / Add Filter Capacitors

在电源输入端并联大电容:

```
    电源 V+ ────┬────────→ 舵机电源
                │
           ┌────┴────┐
           │ 1000uF  │  ← 电解电容 (耐压 >= 10V)
           │  16V    │
           └────┬────┘
                │
    电源 GND ───┴────────→ 公共地
```

- 1000uF 电容 (Electrolytic, >= 10V) 跨接在 V+ 和 GND 之间
- 可以再加一个 0.1uF 陶瓷电容 (滤高频噪声)
- 这只能缓解瞬态电流, 不能替代独立电源!

#### 方案 C: 分时启动舵机 / Stagger Servo Activation

```cpp
   // 不要同时启动所有舵机 / Don't start all servos at once

   // 错误 / WRONG:
   for (int i = 1; i <= 4; i++) {
     st.WritePosEx(i, 2048, 2000, 50); // 同时启动!
   }

   // 正确 / CORRECT:
   for (int i = 1; i <= 4; i++) {
     st.WritePosEx(i, 2048, 2000, 50);
     delay(100); // 错开 100ms
   }
```

### 验证步骤 / Verification Steps

1. 用万用表测量外部电源输出电压 (空载时应在 5.0V-6.0V)
2. 舵机运动时再次测量 (压降不应超过 0.3V)
3. 观察串口输出, 确认不再出现 "rst:0x10" 等重启信息

---

## 2. 摄像头不工作 / Camera Not Working

### 症状 / Symptoms

- `camera_init()` 返回错误码
- 拍摄的图像全黑或全绿
- 串口输出 "Camera probe failed" 或 "OV2640 not detected"

### 排查步骤 / Troubleshooting Steps

#### Step 1: 确认是 Sense 版本 / Verify Sense Version

```
    检查 XIAO 板上是否有摄像头模块:

    正确 (Sense 版本):              错误 (普通版):
    ┌──────────────┐               ┌──────────────┐
    │ ┌──────────┐ │               │              │
    │ │ 摄像头模块│ │  ← 有这个     │   (空)        │  ← 没有摄像头
    │ │ OV2640   │ │               │              │
    │ └──────────┘ │               └──────────────┘
    │              │
    │ 旁边还有 PDM │
    │ 麦克风芯片   │
    └──────────────┘

    如果没有摄像头模块 → 你买错版本了, 需要购买 Sense 版本
```

#### Step 2: 启用 PSRAM / Enable PSRAM

摄像头需要 PSRAM 来存储图像数据. 在 Arduino IDE 中:

| 设置 / Setting | 正确值 / Correct Value |
|---|---|
| PSRAM | **`OPI PSRAM`** (必须!) |
| Partition Scheme | `Huge APP (3MB)` (推荐) |
| Flash Size | `8MB` |

```
    Arduino IDE 菜单路径:
    Tools → PSRAM → OPI PSRAM

    ⚠️ 如果 PSRAM 设为 "Disabled", 摄像头初始化会失败!
    ⚠️ If PSRAM is "Disabled", camera init WILL fail!
```

#### Step 3: 检查摄像头初始化配置 / Check Camera Init Config

```cpp
   // XIAO ESP32S3 Sense 摄像头引脚配置
   // XIAO ESP32S3 Sense camera pin configuration

   camera_config_t config;
   config.ledc_channel = LEDC_CHANNEL_0;
   config.ledc_timer   = LEDC_TIMER_0;
   config.pin_d0       = 15;  // GPIO15
   config.pin_d1       = 17;  // GPIO17
   config.pin_d2       = 18;  // GPIO18
   config.pin_d3       = 16;  // GPIO16
   config.pin_d4       = 14;  // GPIO14
   config.pin_d5       = 12;  // GPIO12
   config.pin_d6       = 11;  // GPIO11
   config.pin_d7       = 10;  // GPIO10 // was incorrect, check
   config.pin_xclk     = 47;  // GPIO47
   config.pin_pclk     = 13;  // GPIO13
   config.pin_vsync    = 21;  // GPIO21
   config.pin_href     = 38;  // GPIO38
   config.pin_sccb_sda = 40;  // GPIO40 (SIOD)
   config.pin_sccb_scl = 39;  // GPIO39 (SIOC)
   config.pin_pwdn     = -1;  // 不使用
   config.pin_reset    = -1;  // 不使用
   config.xclk_freq_hz = 20000000;
   config.pixel_format = PIXFORMAT_JPEG;
   config.frame_size   = FRAMESIZE_QVGA; // 320x240, 足够 AI 识别
   config.jpeg_quality = 12;
   config.fb_count     = 2;   // 需要 PSRAM

   // 重要: 必须在 PSRAM 可用时使用 fb_count=2
   if (psramFound()) {
     config.frame_size = FRAMESIZE_QVGA;
     config.jpeg_quality = 10;
     config.fb_count = 2;
     Serial.println("PSRAM found - using dual buffer");
   } else {
     config.frame_size = FRAMESIZE_QVGA;
     config.jpeg_quality = 12;
     config.fb_count = 1;
     Serial.println("WARNING: No PSRAM - single buffer only");
   }
```

#### Step 4: 检查内存 / Check Memory

```cpp
   void checkMemory() {
     Serial.print("Free heap: ");
     Serial.print(ESP.getFreeHeap());
     Serial.print(" bytes");

     if (psramFound()) {
       Serial.print(" | Free PSRAM: ");
       Serial.print(ESP.getFreePsram());
       Serial.print(" bytes");
     } else {
       Serial.print(" | PSRAM: NOT FOUND!");
     }
     Serial.println();
   }
```

- 摄像头需要至少 ~100KB 连续内存
- 如果 Free Heap < 100KB, 检查是否有其他占用大量内存的代码
- 确认 PSRAM 已启用 (应显示 "Free PSRAM: ~8MB")

### 常见错误码 / Common Error Codes

| 错误码 / Error | 含义 / Meaning | 解决方案 / Solution |
|---|---|---|
| `ESP_ERR_CAMERA_NOT_DETECTED` | 找不到摄像头 | 确认 Sense 版本; 检查排线连接 |
| `ESP_ERR_NO_MEM` | 内存不足 | 启用 PSRAM; 减小帧大小; 降低 fb_count |
| `ESP_FAIL` | 通用失败 | 检查 PSRAM; 检查引脚配置; 降低 XCLK 频率 |

---

## 3. 没有声音 / No Sound from Speaker

### 排查步骤 / Troubleshooting Steps

#### Step 1: 检查接线 / Check Wiring

```
    MAX98357A 引脚对照:

    MAX98357A ────→ XIAO
    ─────────────────────
    VIN     ────→ 5V          ⚠️ 不是 3.3V!
    GND     ────→ GND
    BCLK    ────→ D3 (GPIO4)
    LRC     ────→ D2 (GPIO3)
    DIN     ────→ D9 (GPIO8)

    喇叭 ────→ MAX98357A
    ─────────────────────
    红(+)   ────→ Speaker+
    黑(-)   ────→ Speaker-
```

常见接线错误:
- BCLK/LRC/DIN 接错引脚 (最容易出错)
- VIN 接到 3.3V 而不是 5V
- 喇叭正负极短路

#### Step 2: 检查喇叭 / Check Speaker

```cpp
   // 用万用表测量喇叭直流电阻
   // Measure speaker DC resistance with multimeter

   // 正常: ~4Ω (±20%)
   // 异常: 0Ω (短路) / ∞ (断路) / 远大于 4Ω

   4Ω 喇叭 → 万用表应显示约 3.2Ω - 4.8Ω
   如果显示 0Ω → 喇叭短路, 需要更换
   如果显示 OL/∞ → 喇叭线圈断路, 需要更换
```

#### Step 3: 检查 I2S 配置 / Check I2S Configuration

```cpp
   // 确认 I2S 引脚和参数正确
   i2s_config_t i2s_config = {
     .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),  // TX 模式
     .sample_rate = 22050,    // 常用采样率
     .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
     .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,  // 单声道
     .communication_format = I2S_COMM_FORMAT_STAND_I2S,
     // ...
   };
```

常见配置错误:
- `I2S_MODE_RX` 而不是 `I2S_MODE_TX` (应该是发送模式)
- `I2S_CHANNEL_FMT_ONLY_RIGHT` 而不是 `ONLY_LEFT` (MAX98357A 默认左声道)
- 采样率过高导致输出异常

#### Step 4: 调整音量 / Adjust Volume

```
    MAX98357A 增益设置 (GAIN 引脚):

    当前增益太小? 尝试:
    ┌────────────────┬─────────────┐
    │ GAIN 连接方式   │ 增益         │
    ├────────────────┬─────────────┤
    │ 悬空 (默认)     │ 9 dB       │
    │ 接 GND         │ 6 dB       │
    │ 接 VIN (5V)    │ 12 dB      │  ← 声音更大
    │ 100kΩ 到 GND  │ 15 dB      │  ← 最大
    └────────────────┴─────────────┘

    也可以在代码中降低采样值来提高音量:
    buf[j] = (int16_t)(sin(...) * 16000);  // 增大振幅系数
```

#### Step 5: I2S 引脚冲突检查 / I2S Pin Conflict Check

```
    确保 I2S 引脚没有与 SPI/I2C/UART 冲突:

    I2S:  D2/GPIO3, D3/GPIO4, D9/GPIO8
    SPI:  D0/GPIO41, D1/GPIO42, D8/GPIO9, D10/GPIO7
    I2C:  D4/GPIO5, D5/GPIO6
    UART: D6/GPIO43, D7/GPIO44

    ✅ 所有引脚分配不重复 → 没有冲突
    如果你有重复, 参照 wiring-guide.md 重新分配
```

---

## 4. STS3032 总线舵机不响应 / STS3032 Servos Not Responding

### 排查步骤 / Troubleshooting Steps

#### Step 1: 检查舵机 ID / Check Servo IDs

```
    每个 STS3032 必须有唯一的 ID (1-4):

    出厂默认 ID 通常都是 1!
    如果不修改, 多个舵机会同时响应同一个指令

    使用 TTL 适配器 + 上位机软件:
    1. 只连接一个舵机到适配器
    2. 打开上位机软件 (如 FEETECH 调试软件)
    3. 扫描当前 ID
    4. 修改为目标 ID (1/2/3/4)
    5. 断电, 换下一个舵机, 重复以上步骤
```

#### Step 2: 检查 TX/RX 是否接反 / Check TX/RX Swap

```
    这是最常见的接线错误!

    XIAO TX (D6/GPIO43) ──→ STS3032 数据线
    XIAO RX (D7/GPIO44) ──→ STS3032 数据线

    ⚠️ STS3032 是半双工 TTL 总线, TX 和 RX 共用一根数据线!
    如果你的转接方式是全双工, 需要合并 TX 和 RX:

    方案 1: 使用总线缓冲芯片 (如 74HC126)
    ┌─────────┐
    │  XIAO   │
    │  TX     ├────┐
    │         │    ├──── 74HC126 ────→ STS3032 数据线
    │  RX     ├────┘
    └─────────┘

    方案 2: 直接连接 + 串联电阻 (简单方案)
    XIAO TX ──→ 100Ω ──┬──→ STS3032 数据线
    XIAO RX ───────────┘

    方案 3: 使用 SoftwareSerial 单线模式 (如果库支持)
```

#### Step 3: 检查波特率 / Check Baud Rate

```cpp
   // STS3032 默认波特率是 1000000 (1Mbps)
   // STS3032 default baud rate is 1000000 (1Mbps)

   Serial1.begin(1000000, SERIAL_8N1, 44, 43);

   // 如果你不确定波特率, 依次尝试:
   // If unsure about baud rate, try in order:
   long bauds[] = {1000000, 500000, 250000, 115200, 57600, 9600};

   for (int i = 0; i < 6; i++) {
     Serial1.begin(bauds[i], SERIAL_8N1, 44, 43);
     delay(100);
     int pos = st.ReadPos(1); // 尝试读取 ID=1 的位置
     if (pos >= 0) {
       Serial.print("Working baud rate: ");
       Serial.println(bauds[i]);
       break;
     }
     Serial1.end();
   }
```

#### Step 4: 检查电源 / Check Power

```
    STS3032 需要独立的 6V 电源 (推荐):

    外部电源 6V ──→ STS3032 VCC (红色线)
    外部电源 GND ──→ 公共地

    用万用表检查:
    - 空载时 VCC: 应为 6V ± 0.3V
    - 舵机运动时: 压降不超过 0.5V
    - 如果电压跌落到 4.5V 以下 → 电源容量不足

    ⚠️ 不要从 XIAO 5V 引脚给 STS3032 供电!
    ⚠️ Do NOT power STS3032 from XIAO 5V pin!
```

#### Step 5: 逐个测试 / Test One by One

```
    如果串联的舵机不工作, 逐个排查:

    1. 只连接 ID=1 的舵机 → 测试通信
    2. 连接 ID=1 + ID=2 → 测试
    3. 连接 ID=1 + ID=2 + ID=3 → 测试
    4. 连接全部 4 个 → 测试

    如果在某一步失败 → 检查新加入的舵机:
    - ID 是否正确
    - 串联接线是否牢固
    - 该舵机是否损坏
```

### STS3032 通信调试代码 / STS3032 Communication Debug Code

```cpp
   void debugSTS3032() {
     Serial.println("=== STS3032 Debug ===");

     // 1. 检查串口初始化
     Serial1.begin(1000000, SERIAL_8N1, 44, 43);
     Serial.print("Serial1 initialized");

     // 2. 扫描所有 ID
     for (int id = 1; id <= 4; id++) {
       int pos = st.ReadPos(id);
       if (pos >= 0) {
         Serial.print("  Servo ID ");
         Serial.print(id);
         Serial.print(": position = ");
         Serial.println(pos);
       } else {
         Serial.print("  Servo ID ");
         Serial.print(id);
         Serial.println(": NO RESPONSE");
       }
     }

     // 3. 检查 PSRAM (舵机库可能需要)
     Serial.print("Free heap: ");
     Serial.println(ESP.getFreeHeap());
   }
```

---

## 5. 屏幕空白 / Screen Blank

### 排查步骤 / Troubleshooting Steps

#### Step 1: 检查 SPI 接线 / Check SPI Wiring

```
    ST7789 SPI 接线 (逐根检查):

    ST7789  ────→  XIAO        线颜色建议
    ─────────────────────────────────────
    VCC     ────→  3.3V         红色
    GND     ────→  GND          黑色
    CS      ────→  D0 (GPIO41)  橙色
    DC      ────→  D1 (GPIO42)  黄色
    RST     ────→  EN/RST       紫色
    SCK     ────→  D8 (GPIO9)   绿色
    MOSI    ────→  D10 (GPIO7)  蓝色
    BL      ────→  3.3V         红色

    常见错误:
    ✗ VCC 接到 5V (ST7789 某些版本不支持 5V)
    ✗ CS 和 DC 接反
    ✗ SCK 和 MOSI 接反
    ✗ RST 忘记连接
    ✗ BL (背光) 没接 → 屏幕有显示但看不见
```

#### Step 2: 检查 CS 和 DC 引脚 / Check CS and DC Pins

```cpp
   // CS 和 DC 是最容易接错的两个引脚:
   // CS and DC are the two most commonly swapped pins:

   // 如果使用 TFT_eSPI, 在 User_Setup.h 中:
   #define TFT_CS   41   // D0/GPIO41 - 片选
   #define TFT_DC   42   // D1/GPIO42 - 数据/命令

   // 如果使用 Adafruit_ST7789:
   Adafruit_ST7789 tft = Adafruit_ST7789(
     41,  // CS
     42,  // DC
     -1   // RST (使用 EN 自动复位)
   );
```

#### Step 3: 检查背光 / Check Backlight

```
    如果屏幕看起来完全黑, 但你不确定是不是真的没显示:

    1. 在强光下斜看屏幕 → 如果能看到很暗的内容 → 背光问题
    2. 用万用表测量 BL 引脚电压 → 应为 3.3V
    3. 如果 BL 悬空 → 接到 3.3V

    有些 ST7789 模块没有 BL 引脚 (背光默认开启)
    有些模块需要 BL 接高电平才会亮
```

#### Step 4: 测试 SPI 通信 / Test SPI Communication

```cpp
   // SPI 回环测试: 向屏幕发送命令, 观察是否有反应
   // SPI loop test: Send commands, observe response

   #include <SPI.h>

   void setup() {
     Serial.begin(115200);
     SPI.begin(9, -1, 7, 41); // SCK, MISO, MOSI, CS

     pinMode(41, OUTPUT); // CS
     pinMode(42, OUTPUT); // DC
     pinMode(-1, OUTPUT); // RST (using EN)

     Serial.println("SPI test: sending SWRESET command...");

     // 软件复位命令 / Software reset command
     digitalWrite(41, LOW);  // CS low
     digitalWrite(42, LOW);  // DC low (command mode)
     SPI.transfer(0x01);     // SWRESET
     digitalWrite(41, HIGH); // CS high

     delay(150); // 等待复位完成 / Wait for reset
     Serial.println("SWRESET sent. If screen flickered, SPI is working.");
   }

   void loop() {}
```

#### Step 5: 检查屏幕分辨率和驱动 / Check Resolution and Driver

```
    确认你的屏幕参数:

    本项目使用:
    - 驱动: ST7789
    - 分辨率: 170 x 320 (1.83 英寸)

    如果你买的是 1.54 英寸版本:
    - 分辨率: 240 x 240
    - 需要修改代码中的分辨率参数

    如果你买的是 1.3 英寸版本:
    - 分辨率: 240 x 240
    - 可能是 SH1106 驱动而不是 ST7789

    确认方法: 查看屏幕模块背面的丝印文字
```

---

## 6. Wi-Fi 连接失败 / Wi-Fi Connection Fails

### 排查步骤 / Troubleshooting Steps

#### Step 1: 检查 SSID 和密码 / Check SSID and Password

```cpp
   // 常见错误:
   // Common mistakes:

   // ✗ SSID 大小写错误
   const char* ssid = "MyWifi";     // 正确
   const char* ssid = "mywifi";     // 错误 (大小写敏感!)
   const char* ssid = " MyWifi";    // 错误 (前面有空格!)
   const char* ssid = "MyWifi ";    // 错误 (后面有空格!)

   // ✗ 密码错误
   const char* password = "12345678";  // 确认密码正确

   // 调试代码:
   WiFi.begin(ssid, password);
   Serial.print("Connecting to: ");
   Serial.println(ssid);

   int attempts = 0;
   while (WiFi.status() != WL_CONNECTED && attempts < 40) {
     delay(500);
     Serial.print(".");
     attempts++;
   }

   if (WiFi.status() == WL_CONNECTED) {
     Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
   } else {
     Serial.println("\nFAILED! Status: " + String(WiFi.status()));
   }
```

#### Step 2: 确认在同一网络 / Verify Same Network

```
    电脑和 XIAO 必须在同一个 Wi-Fi 网络上!

    如果你有双频路由器 (2.4GHz + 5GHz):
    - ESP32S3 只支持 2.4GHz
    - 确保电脑和 ESP32 连接到同一个频段
    - 或者确保路由器允许 2.4GHz 和 5GHz 设备互通

    检查方法:
    - 电脑 IP: 打开命令行 → ipconfig → 查看 IPv4 地址
    - XIAO IP: 串口输出 WiFi.localIP()
    - 两者应在同一子网 (如 192.168.1.x)
```

#### Step 3: Wi-Fi 状态码 / Wi-Fi Status Codes

| 状态码 / Code | 含义 / Meaning | 解决方案 / Solution |
|---|---|---|
| 0 | `WL_IDLE_STATUS` | 正在连接中, 继续等待 |
| 1 | `WL_NO_SSID_AVAIL` | 找不到该 SSID | 确认 SSID 拼写; 确认路由器开启 |
| 4 | `WL_CONNECT_FAILED` | 连接失败 | 密码错误; 信号太弱 |
| 6 | `WL_DISCONNECTED` | 已断开 | 检查距离; 重启路由器 |
| 3 | `WL_CONNECTED` | 已连接 | 一切正常! |

#### Step 4: 信号强度 / Signal Strength

```cpp
   void checkWiFiSignal() {
     if (WiFi.status() == WL_CONNECTED) {
       long rssi = WiFi.RSSI();
       Serial.print("WiFi RSSI: ");
       Serial.print(rssi);
       Serial.print(" dBm - ");

       if (rssi > -50)      Serial.println("Excellent");
       else if (rssi > -60) Serial.println("Good");
       else if (rssi > -70) Serial.println("Fair");
       else                 Serial.println("Weak (may cause issues)");
     }
   }
```

---

## 7. 舵机抖动 / Servo Jitter

### 排查步骤 / Troubleshooting Steps

#### Step 1: 检查电源质量 / Check Power Quality

```
    舵机抖动最常见的原因是电源纹波过大:

    用示波器或万用表 (AC 档) 测量:
    - 电源 V+ 的纹波应 < 100mV
    - 如果纹波过大 → 增加滤波电容

    解决方案:
    V+ ───┬──── 1000uF ────┬──── GND
          │                │
          └──── 0.1uF ────┘

    大电容 (1000uF): 滤低频纹波
    小电容 (0.1uF): 滤高频噪声
```

#### Step 2: 调整 PWM 频率 / Adjust PWM Frequency

```cpp
   // PCA9685 默认频率 50Hz 适合大多数舵机
   // 但某些舵机可能在 60Hz 表现更好

   pwm.setPWMFreq(50);  // 标准: 50Hz (20ms 周期)
   // 如果抖动, 尝试:
   // pwm.setPWMFreq(60);  // 某些舵机更好
   // pwm.setPWMFreq(40);  // 更低的频率

   // 对于微型舵机 (2g/3.7g), 50Hz 通常是最佳
   // 对于 MG90S, 50Hz 也是标准
```

#### Step 3: 检查 PWM 范围 / Check PWM Range

```cpp
   // 不同舵机的 PWM 范围不同:

   // SG90 / MG90S (标准):
   //   0° → pulse ~500us  → PCA9685 tick = 500/((1/50)*4096*1000) ≈ 102
   //   180° → pulse ~2500us → PCA9685 tick ≈ 512
   //   实际范围: MIN=102, MAX=512

   // 微型 2g 舵机:
   //   范围可能不同, 需要实测
   //   从中间值开始: 300

   // 如果设置了超出范围的 PWM 值 → 舵机会抖动或发出异响
   // 建议先找到舵机的实际范围:

   void findServoRange(int channel) {
     Serial.println("Sweeping servo to find range...");
     for (int pulse = 100; pulse <= 600; pulse += 10) {
       pwm.setPWM(channel, 0, pulse);
       Serial.print("Pulse: ");
       Serial.print(pulse);
       delay(50);
     }
     // 观察舵机何时开始/停止移动, 记录范围
   }
```

#### Step 4: 减少同时运动的舵机数量 / Reduce Simultaneous Servo Movement

```
    多个舵机同时运动会拉低电源电压 → 导致抖动

    建议:
    - 同一时刻最多 2 个舵机在运动
    - 使用分时控制, 舵机之间错开 20-50ms
    - 降低舵机速度 (减慢位置变化)

    // 错开舵机运动:
    void smoothMove(int channel, int from, int to, int step_delay) {
      int step = (to > from) ? 1 : -1;
      for (int p = from; p != to; p += step) {
        pwm.setPWM(channel, 0, p);
        delay(step_delay);  // 2-5ms per step for smooth movement
      }
    }
```

---

## 8. 上传失败 / Upload Fails

### 症状 / Symptoms

- Arduino IDE 显示 "Failed to connect to ESP32"
- 上传卡在 "Connecting........_____....."
- 显示 "A fatal error occurred: No serial data received"

### 排查步骤 / Troubleshooting Steps

#### Step 1: 检查 USB 线 / Check USB Cable

```
    ⚠️ 这是上传失败最常见的原因!

    很多 USB 线只能充电, 不能传输数据!

    测试方法:
    - 插上 USB 线 → Windows 设备管理器 → 端口 (COM & LPT)
    - 如果出现新的 COM 端口 → 数据线 OK
    - 如果没有新 COM 端口 → 换一根 USB 数据线

    推荐: 使用带有数据传输功能的 Type-C 数据线
    Avoid: 手机附赠的 "快充线" (很多只能充电)
```

#### Step 2: 手动进入 BOOT 模式 / Manual BOOT Mode

```
    XIAO ESP32S3 进入下载模式:

    1. 按住 BOOT 键 (不松开)
       ┌──────────┐
       │  BOOT ●  │ ← 按住
       │          │
       │  EN ●    │
       └──────────┘

    2. 点击 EN 键 (按一下松开)
       ┌──────────┐
       │  BOOT ●  │ ← 仍然按住
       │          │
       │  EN ●    │ ← 按一下松开
       └──────────┘

    3. 松开 BOOT 键

    4. 点击 Arduino IDE 的 "上传"

    现在应该可以正常上传了!
```

#### Step 3: 检查驱动 / Check Driver

```
    Windows 设备管理器检查:

    ✅ 正常:
    端口 (COM & LPT)
      ├── USB 串行设备 (COM3)     ← XIAO
      └── ...

    ❌ 异常:
    其他设备
      ├── USB2.0-Serial            ← 驱动未安装
      └── ...

    解决方案:
    - 安装 CH340 驱动 (WCH 官网下载)
    - 或者安装 CP210x 驱动 (Silicon Labs 官网下载)
    - 安装后重启电脑
    - 重新插拔 USB 线
```

#### Step 4: 检查分区方案 / Check Partition Scheme

```
    Arduino IDE 分区方案设置:

    Tools → Partition Scheme →

    推荐选择 / Recommended:
    ┌────────────────────────────────┐
    │ "Huge APP (3MB)"              │  ← 推荐! 代码空间大
    │ "Default 4MB with spiffs"     │  ← 也可以
    │ "Default"                     │  ← 代码可能太大装不下
    └────────────────────────────────┘

    如果代码较大 (> 1MB), 选择 "Huge APP (3MB)"
    否则可能出现 "Flash overflow" 错误
```

#### Step 5: 检查 Flash Size / Check Flash Size

```
    XIAO ESP32S3 Sense: 8MB Flash

    Arduino IDE 设置:
    Tools → Flash Size → "8MB"

    如果选了错误的 Flash Size → 上传可能失败或运行异常
```

#### Step 6: 清除 Flash / Erase Flash

```
    如果以上都不管用, 尝试完全清除 Flash:

    Arduino IDE 设置:
    Tools → Erase All Flash Before Sketch Upload → "Enabled"

    然后重新上传

    这会清除所有数据和 WiFi 设置, 但可以解决顽固的上传问题
```

### 上传失败错误信息速查 / Upload Error Quick Reference

| 错误信息 / Error Message | 原因 / Cause | 解决方案 / Solution |
|---|---|---|
| `No serial data received` | COM 端口选择错误 / USB 线问题 | 检查 COM 端口; 换 USB 数据线; 手动 BOOT 模式 |
| `Failed to connect to ESP32` | ESP32 未进入下载模式 | 手动 BOOT 模式 (见 Step 2) |
| `Flash overflow` | 代码太大 | 改分区方案为 "Huge APP"; 删减代码 |
| `Timed out waiting for packet header` | 连接不稳定 | 换 USB 口; 换 USB 线; 关闭其他串口程序 |
| `Invalid header` | 波特率不匹配 / Flash 损坏 | 尝试降低 Upload Speed; Erase Flash |
| `A fatal error occurred` | 通用错误 | 综合以上所有步骤排查 |

---

## 附录: 调试工具箱 / Appendix: Debug Toolkit

### 串口监视器常用命令 / Useful Serial Commands

```cpp
   // 在你的代码中添加调试菜单:
   void handleSerialCommand() {
     if (Serial.available()) {
       String cmd = Serial.readStringUntil('\n');
       cmd.trim();

       if (cmd == "heap") {
         Serial.printf("Free heap: %d, Free PSRAM: %d\n",
           ESP.getFreeHeap(), ESP.getFreePsram());
       }
       else if (cmd == "wifi") {
         Serial.printf("WiFi: %s, RSSI: %d dBm, IP: %s\n",
           WiFi.SSID().c_str(), WiFi.RSSI(),
           WiFi.localIP().toString().c_str());
       }
       else if (cmd == "i2c") {
         scanI2C();  // 运行 I2C 扫描
       }
       else if (cmd.startsWith("servo ")) {
         int id = cmd.substring(6).toInt();
         Serial.printf("Servo %d pos: %d\n", id, st.ReadPos(id));
       }
       else if (cmd == "restart") {
         Serial.println("Restarting...");
         ESP.restart();
       }
       else if (cmd == "help") {
         Serial.println("Commands: heap, wifi, i2c, servo <id>, restart");
       }
     }
   }
```

### 模块测试优先级 / Module Test Priority

```
    如果系统不工作, 按以下顺序逐个排查:

    1. XIAO 基础功能 (LED + 串口)    ← 最基本, 先确认
    2. 电源电压 (万用表测量)           ← 很多问题的根源
    3. I2C 扫描 (找 PCA9685)          ← 确认 I2C 总线
    4. SPI 屏幕 (显示测试图案)         ← 确认 SPI 总线
    5. I2S 音频 (播放测试音)           ← 确认 I2S 总线
    6. UART 总线舵机 (读取位置)        ← 确认 UART 通信
    7. 摄像头 (拍摄测试图像)           ← 确认摄像头
    8. WiFi 连接                      ← 确认网络

    逐个确认, 不要跳步!
    Test one by one, don't skip steps!
```

### 有用的串口输出格式 / Useful Serial Output Format

```cpp
   void printSystemStatus() {
     Serial.println("╔══════════════════════════════╗");
     Serial.println("║     AICat System Status      ║");
     Serial.println("╠══════════════════════════════╣");
     Serial.printf("║ Heap:  %6d bytes          ║\n", ESP.getFreeHeap());
     Serial.printf("║ PSRAM: %6d bytes          ║\n", ESP.getFreePsram());
     Serial.printf("║ WiFi:  %-20s ║\n",
       WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
     Serial.printf("║ Uptime: %lu seconds         ║\n", millis()/1000);
     Serial.println("╚══════════════════════════════╝");
   }
```
