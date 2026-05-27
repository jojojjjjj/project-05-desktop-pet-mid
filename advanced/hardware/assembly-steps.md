# AICat 组装步骤 / AICat Assembly Steps

> Advanced Track - Desktop Pet Hardware
> 高级赛道 - 桌面宠物硬件

---

## 概述 / Overview

按照以下步骤逐步组装和测试你的 AICat. 每一步都包含测试方法, 确保问题在早期发现.

Follow these steps to assemble and test your AICat incrementally. Each step includes test procedures to catch problems early.

```
    步骤流程 / Step Flow:

    Step 1        Step 2        Step 3        Step 4        Step 5
    XIAO    ──→  ST7789   ──→  PCA9685  ──→  MAX98357A ──→ STS3032
    基础测试      屏幕测试      舵机驱动       音频测试       总线舵机
      │              │              │              │              │
      ▼              ▼              ▼              ▼              ▼
    Step 6        Step 7        Step 8        Step 9
    完整接线  ──→  3D打印   ──→  安装    ──→  最终测试
    面包板        猫咪外壳      电子部件       整机验收
```

---

## Step 1: 测试 XIAO ESP32S3 Sense 基础功能 / Test XIAO Basic Functions

### 目标 / Goal
确认 XIAO ESP32S3 Sense 可以正常上传代码, LED 闪烁, 串口输出正常.

### 操作步骤 / Steps

1. **安装驱动 / Install Driver**
   - 将 XIAO 通过 USB-C 线连接到电脑
   - Windows 应自动识别 (CH340/CP2102 驱动), 如未识别, 手动安装驱动
   - 在 Arduino IDE 中安装 `esp32` by Espressif Systems 开发板支持
   - 选择开发板: `XIAO_ESP32S3`

2. **上传 Blink 测试 / Upload Blink Test**
   ```cpp
   // Arduino IDE: File → Examples → 01.Basics → Blink
   // 或使用以下代码 / Or use this code:

   void setup() {
     pinMode(LED_BUILTIN, OUTPUT);
     Serial.begin(115200);
   }

   void loop() {
     digitalWrite(LED_BUILTIN, HIGH);
     Serial.println("LED ON");
     delay(1000);
     digitalWrite(LED_BUILTIN, LOW);
     Serial.println("LED OFF");
     delay(1000);
   }
   ```

3. **验证 / Verify**
   - LED 应每秒闪烁一次 / LED blinks once per second
   - 打开串口监视器 (115200 baud), 看到 "LED ON" / "LED OFF" 循环输出

### Arduino IDE 设置 / Arduino IDE Settings

| 设置项 / Setting | 值 / Value |
|---|---|
| 开发板 / Board | `XIAO_ESP32S3` |
| Upload Mode | `Default` 或 `UART0 / Hardware CDC` |
| Flash Size | `8MB` |
| Partition Scheme | `Default` 或 `Huge APP (3MB)` |
| PSRAM | `OPI PSRAM` (必须启用!) |
| Port | 选择对应的 COM 端口 |

### 故障排除 / Troubleshooting

| 问题 / Problem | 解决方案 / Solution |
|---|---|
| 找不到 COM 端口 | 安装 CH340 驱动; 尝试换 USB 线 (有些线只能充电); 尝试按住 BOOT 键再插 USB |
| 上传失败 | 按住 BOOT 键, 点击上传, 等待 "Connecting..." 后松开 BOOT |
| LED 不亮 | 确认选择的是 `XIAO_ESP32S3` 而不是 `XIAO_ESP32S3 Sense` (某些版本名称不同) |
| 串口无输出 | 检查波特率是否为 115200; 确认使用了数据线而非充电线 |

---

## Step 2: 接线并测试 ST7789 屏幕 / Wire and Test ST7789 Screen

### 目标 / Goal
让 ST7789 LCD 显示测试图案 (彩色条纹或文字).

### 操作步骤 / Steps

1. **断电接线 / Power Off and Wire**
   - 拔掉 XIAO 的 USB 线
   - 按照接线指南连接 ST7789 (见 wiring-guide.md)
   - 仔细检查每根线, 特别是 VCC 接 3.3V

2. **安装库 / Install Library**
   - Arduino IDE: 安装 `TFT_eSPI` by Bodmer
   - 或者安装 `Adafruit ST7789` + `Adafruit GFX`

3. **上传测试代码 / Upload Test Code**

   使用 TFT_eSPI (推荐):
   ```cpp
   #include <TFT_eSPI.h>

   TFT_eSPI tft = TFT_eSPI();

   void setup() {
     tft.init();
     tft.setRotation(1); // 横屏
     tft.fillScreen(TFT_RED);
     delay(500);
     tft.fillScreen(TFT_GREEN);
     delay(500);
     tft.fillScreen(TFT_BLUE);
     delay(500);
     tft.fillScreen(TFT_BLACK);
     tft.setCursor(10, 10);
     tft.setTextColor(TFT_WHITE);
     tft.setTextSize(2);
     tft.println("AICat Ready!");
   }

   void loop() {}
   ```

   如果使用 TFT_eSPI, 需要修改 `User_Setup.h`:
   ```
   #define ST7789_DRIVER
   #define TFT_WIDTH  170
   #define TFT_HEIGHT 320
   #define TFT_CS   41
   #define TFT_DC   42
   #define TFT_RST  -1  // 使用 EN 复位
   #define TFT_SCLK 9
   #define TFT_MOSI 7
   ```

4. **验证 / Verify**
   - 屏幕应依次显示红、绿、蓝, 最后显示 "AICat Ready!" 文字

### 故障排除 / Troubleshooting

| 问题 / Problem | 解决方案 / Solution |
|---|---|
| 屏幕全白/全黑 | 检查接线, 特别是 CS、DC、SCK、MOSI; 确认 VCC 接 3.3V |
| 颜色不对 (红蓝反了) | 修改初始化参数, 设置颜色顺序 (RGB vs BGR) |
| 花屏/闪烁 | 检查杜邦线是否插紧; SPI 线不要太长 |
| 编译错误 | 检查 User_Setup.h 配置是否正确; 确认 TFT_WIDTH/HEIGHT |

---

## Step 3: 接线并测试 PCA9685 (单个舵机) / Wire and Test PCA9685

### 目标 / Goal
通过 PCA9685 控制一个舵机来回转动, 验证 I2C 通信和 PWM 输出.

### 操作步骤 / Steps

1. **断电接线 / Power Off and Wire**
   - 拔掉 XIAO 的 USB 线和外部电源
   - 按照接线指南连接 PCA9685
   - 在 CH14 或 CH15 插入一个 MG90S 舵机 (用于测试)
   - 连接外部 5V/6V 电源到 PCA9685 的 V+ 和 GND
   - 确认外部电源 GND 与 XIAO GND 相连 (公共地!)

2. **安装库 / Install Library**
   - Arduino IDE: 安装 `Adafruit PWM Servo Driver Library`

3. **上传测试代码 / Upload Test Code**
   ```cpp
   #include <Wire.h>
   #include <Adafruit_PWMServoDriver.h>

   Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

   // SG90/MG90S: min=150, max=600 (大约 0°-180°)
   #define SERVO_MIN 150
   #define SERVO_MAX 600

   void setup() {
     Serial.begin(115200);
     pwm.begin();
     pwm.setPWMFreq(50); // 50Hz 标准舵机频率
     Serial.println("PCA9685 Test Ready");
   }

   void loop() {
     // CH14 舵机来回转动
     pwm.setPWM(14, 0, SERVO_MIN);
     Serial.println("Servo -> MIN");
     delay(1000);

     pwm.setPWM(14, 0, SERVO_MAX);
     Serial.println("Servo -> MAX");
     delay(1000);
   }
   ```

4. **验证 / Verify**
   - 舵机应每秒在两个位置之间来回转动
   - 串口输出 "Servo -> MIN" 和 "Servo -> MAX"

### 故障排除 / Troubleshooting

| 问题 / Problem | 解决方案 / Solution |
|---|---|
| I2C 扫描找不到设备 | 运行 I2C Scanner 示例; 检查 SDA/SCL 接线; 确认 PCA9685 VCC 接 3.3V |
| 舵机不动 | 检查外部电源是否开启; 确认 V+ 有电压 (用万用表); 检查舵机线方向 |
| ESP32 重启 | 舵机电流过大! 确认舵机由外部电源供电, 不是从 XIAO 取电 |
| 舵机抖动 | 降低 PWM 频率到 50Hz; 检查电源电压是否稳定; 增加电源滤波电容 |

### I2C 扫描代码 / I2C Scanner Code

```cpp
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin(5, 6); // SDA=GPIO5, SCL=GPIO6
  Serial.println("Scanning I2C...");
}

void loop() {
  int count = 0;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found device at 0x");
      Serial.println(addr, HEX);
      count++;
    }
  }
  Serial.print("Total: ");
  Serial.print(count);
  Serial.println(" device(s)");
  delay(2000);
}
```

> 应找到 0x40 (PCA9685). 如果没找到, 检查接线.
> Should find 0x40 (PCA9685). If not found, check wiring.

---

## Step 4: 接线并测试 MAX98357A 喇叭 / Wire and Test MAX98357A

### 目标 / Goal
通过 I2S 播放一段测试音频, 验证 MAX98357A 和喇叭正常工作.

### 操作步骤 / Steps

1. **断电接线 / Power Off and Wire**
   - 拔掉 XIAO USB 线
   - 按照接线指南连接 MAX98357A 和喇叭
   - MAX98357A VIN 接 5V (XIAO 的 5V 引脚即可, 不需要外部电源)

2. **安装库 / Install Library**
   - Arduino IDE: 安装 `ESP32-audioI2S` by schreibfaul1
   - 或者使用 `driver/i2s.h` (ESP-IDF 内置)

3. **上传测试代码 / Upload Test Code**
   ```cpp
   #include <driver/i2s.h>

   #define I2S_BCLK 4
   #define I2S_LRC  3
   #define I2S_DIN  8

   void setup() {
     Serial.begin(115200);

     i2s_config_t i2s_config = {
       .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
       .sample_rate = 22050,
       .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
       .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
       .communication_format = I2S_COMM_FORMAT_STAND_I2S,
       .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
       .dma_buf_count = 8,
       .dma_buf_len = 1024,
       .use_apll = false,
       .tx_desc_auto_clear = true,
       .fixed_mclk = 0
     };

     i2s_pin_config_t pin_config = {
       .bck_io_num = I2S_BCLK,
       .ws_io_num = I2S_LRC,
       .data_out_num = I2S_DIN,
       .data_in_num = I2S_PIN_NO_CHANGE
     };

     i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
     i2s_set_pin(I2S_NUM_0, &pin_config);

     Serial.println("I2S initialized. Playing test tone...");

     // 播放 1kHz 测试音 / Play 1kHz test tone
     playTone(1000, 1000); // 1kHz, 1 second
     Serial.println("Test complete!");
   }

   void playTone(int freq, int duration_ms) {
     int samples = (22050 * duration_ms) / 1000;
     int16_t buf[256];
     for (int i = 0; i < samples; i += 256) {
       int len = min(256, samples - i);
       for (int j = 0; j < len; j++) {
         double t = (double)(i + j) / 22050.0;
         buf[j] = (int16_t)(sin(2.0 * PI * freq * t) * 8000);
       }
       size_t written;
       i2s_write(I2S_NUM_0, buf, len * 2, &written, portMAX_DELAY);
     }
   }

   void loop() {}
   ```

4. **验证 / Verify**
   - 喇叭应发出 1 秒的 1kHz "嘟" 声
   - 如果声音太小, 检查 MAX98357A 的 GAIN 设置

### 故障排除 / Troubleshooting

| 问题 / Problem | 解决方案 / Solution |
|---|---|
| 完全没有声音 | 检查接线 (BCLK/LRC/DIN); 确认喇叭线没短路; 用万用表检查喇叭阻抗 (~4Ω) |
| 声音太小 | MAX98357A GAIN 引脚悬空 = 9dB; 可将 GAIN 接 VCC 获得 12dB |
| 噪音/杂音 | 检查喇叭线是否接触不良; I2S 线远离舵机电源线 |
| 编译错误 | 确认 ESP32 开发板版本 >= 2.0.0; 检查 I2S 引脚定义 |

---

## Step 5: 接线并测试 STS3032 总线舵机 / Wire and Test STS3032 Bus Servos

### 目标 / Goal
与 STS3032 总线舵机通信, 读取舵机位置, 控制舵机转动.

### 前置条件 / Prerequisites
- 已使用 TTL 适配器将 4 个 STS3032 舵机的 ID 分别设置为 1, 2, 3, 4

### 操作步骤 / Steps

1. **断电接线 / Power Off and Wire**
   - 拔掉 XIAO USB 线和外部电源
   - 按照接线指南连接 STS3032 总线
   - 连接外部 6V 电源到 STS3032 VCC 和 GND
   - 确认外部电源 GND 与 XIAO GND 相连

2. **安装库 / Install Library**
   - 安装 STS3032 兼容库 (如 `SCServo` by Feetech)
   - 或者使用原始串口命令 (参考 STS3032 通信协议手册)

3. **上传测试代码 / Upload Test Code**
   ```cpp
   #include <SCServo.h>

   SMS_STS st;

   void setup() {
     Serial.begin(115200);
     Serial1.begin(1000000, SERIAL_8N1, 44, 43); // RX=GPIO44, TX=GPIO43

     st.pSerial = &Serial1;
     delay(1000);

     Serial.println("STS3032 Test Ready");

     // 读取 ID=1 的舵机位置
     int pos = st.ReadPos(1);
     Serial.print("Servo 1 position: ");
     Serial.println(pos);

     // 移动 ID=1 的舵机
     st.WritePosEx(1, 2048, 2000, 50); // 位置2048, 速度2000, 加速度50
     Serial.println("Servo 1 -> position 2048");
     delay(2000);

     st.WritePosEx(1, 1000, 2000, 50);
     Serial.println("Servo 1 -> position 1000");
     delay(2000);
   }

   void loop() {}
   ```

4. **验证 / Verify**
   - 串口输出舵机当前位置
   - ID=1 的舵机在两个位置之间移动

### 故障排除 / Troubleshooting

| 问题 / Problem | 解决方案 / Solution |
|---|---|
| 无响应 | 检查 TX/RX 是否接反; 确认波特率 1000000; 检查舵机 ID |
| 通信不稳定 | 总线线太长; 信号线靠近电源线受干扰; 加终端电阻 |
| 舵机抖动不停 | 舵机处于中位附近正常抖动; 检查电源电压是否足够 |
| 只有一个舵机响应 | 检查其他舵机的 ID 是否正确设置; 串联连接是否正常 |

---

## Step 6: 完整接线 (面包板) / Full Wiring on Breadboard

### 目标 / Goal
将所有模块集成到一块面包板上, 确保所有子系统同时正常工作.

### 操作步骤 / Steps

1. **规划布局 / Plan Layout**
   ```
   面包板建议布局 (从左到右):
   [STS3032接线] [XIAO] [ST7789 LCD]
                        [PCA9685]
                        [MAX98357A + 喇叭]
                        [电源接入点]
   ```

2. **逐步接线 / Wire Step by Step**
   - 先接所有 GND 线 (公共地) -- 用黑色线
   - 再接所有电源线 (3.3V, 5V, 外部 V+) -- 用红色线
   - 然后接 I2C (SDA/SCL) -- 用蓝色线
   - 然后接 SPI (CS/DC/SCK/MOSI) -- 用绿色线
   - 然后接 I2S (BCLK/LRC/DIN) -- 用黄色线
   - 最后接 UART (TX/RX) -- 用白色线

3. **通电测试顺序 / Power-Up Test Order**
   - 只插 XIAO USB (不插外部电源) → 测试屏幕和 I2C 通信
   - 再插外部电源 → 测试舵机动作
   - 运行综合测试程序 → 所有功能同时运行

4. **综合测试代码框架 / Integration Test Skeleton**
   ```cpp
   void loop() {
     // 1. 显示猫咪表情
     displayCatFace();

     // 2. 动耳朵 (PCA9685 CH14/CH15)
     moveEars();

     // 3. 摇尾巴 (PCA9685 CH13)
     wagTail();

     // 4. 腿部动作 (STS3032)
     moveLegs();

     // 5. 发出声音 (MAX98357A)
     playMeow();

     delay(100);
   }
   ```

### 故障排除 / Troubleshooting

| 问题 / Problem | 解决方案 / Solution |
|---|---|
| 多个模块不工作 | 检查公共地; 逐一断开模块排查; 确认没有引脚冲突 |
| SPI 和 I2S 干扰 | 确认引脚分配没有重复; 参考 wiring-guide.md 的完整引脚表 |
| 随机重启 | 电源不足; 增加 1000uF 电容在电源输入端; 确认外部电源容量 >= 3A |

---

## Step 7: 3D 打印猫咪外壳 / 3D Print Cat Shell

### 目标 / Goal
打印 AICat 的外壳部件, 为安装电子元件做准备.

### 操作步骤 / Steps

1. **获取 STL 文件 / Get STL Files**
   - 从项目仓库下载 STL 文件
   - 文件通常包括: 猫头 (头部外壳), 猫身 (身体外壳), 耳朵 x2, 腿 x4

2. **切片设置 / Slicer Settings**

   | 参数 / Parameter | 推荐值 / Recommended |
   |---|---|
   | 材料 / Material | PLA |
   | 层高 / Layer Height | 0.2mm |
   | 填充 / Infill | 15-20% |
   | 壁厚 / Wall Thickness | 2-3 层 |
   | 支撑 / Supports | 是 (头部内侧需要) |
   | 打印温度 / Print Temp | 200-210°C |
   | 底板温度 / Bed Temp | 60°C |

3. **打印并检查 / Print and Inspect**
   - 打印所有部件
   - 检查尺寸是否合适 (舵机孔位, 屏幕开口等)
   - 用砂纸打磨毛刺

4. **试装 / Dry Fit**
   - 不涂胶, 先将所有部件拼合, 确认尺寸合适
   - 将舵机放入预留孔位, 确认安装方式

### 故障排除 / Troubleshooting

| 问题 / Problem | 解决方案 / Solution |
|---|---|
| 尺寸不对 | 检查打印机校准; STL 文件缩放比例; 用游标卡尺测量实际打印尺寸 |
| 舵机装不进去 | 用砂纸/锉刀扩大孔位; 注意不要打磨过量 |
| 打印翘曲 | 使用 Brim/Raft; 增加底板粘合力; 降低首层速度 |
| 屏幕开口对不上 | 可用 Dremel 或锉刀微调; 也可以修改 STL 重新打印 |

---

## Step 8: 安装电子部件到外壳 / Mount Electronics into Shell

### 目标 / Goal
将所有电子元件安装到 3D 打印的猫咪外壳中.

### 操作步骤 / Steps

1. **安装屏幕 / Mount Display**
   - 将 ST7789 LCD 放入猫头的屏幕窗口
   - 用少量热熔胶固定 (不要用太多, 方便日后拆卸)
   - 确认屏幕方向正确

2. **安装耳朵舵机 / Mount Ear Servos**
   - 将 MG90S 舵机放入耳朵安装位
   - 用螺丝或热熔胶固定
   - 安装耳朵外壳到舵机输出轴

3. **安装嘴巴舵机 / Mount Mouth Servo**
   - 将微型舵机放入嘴巴机构
   - 连杆连接舵机与嘴巴活动件

4. **安装尾巴舵机 / Mount Tail Servo**
   - 将微型舵机放入尾巴安装位
   - 连接尾巴活动件

5. **安装腿部总线舵机 / Mount Leg Bus Servos**
   - 将 STS3032 舵机 (ID 1-4) 分别放入四条腿的安装位
   - 连接腿部连杆机构
   - 串联舵机信号线

6. **安装主板和面包板 / Mount Main Board**
   - 将面包板 (或 XIAO + PCA9685) 固定在猫咪身体内部
   - 走线: 电源线、信号线、总线线
   - 安装 MAX98357A 和喇叭

7. **整理线缆 / Cable Management**
   - 用扎带或胶带固定线缆
   - 确保线缆不会影响舵机运动
   - 外部电源线从预留孔引出

### 故障排除 / Troubleshooting

| 问题 / Problem | 解决方案 / Solution |
|---|---|
| 空间不够 | 精简面包板走线; 考虑用定制 PCB; 减少线缆长度 |
| 舵机运动受阻 | 检查舵机行程范围是否超出外壳限制; 调整舵机中位 |
| 线缆被卡住 | 重新走线; 用扎带固定到不动的位置; 预留活动空间 |

---

## Step 9: 最终检查与通电测试 / Final Wiring Check and Power-Up Test

### 目标 / Goal
完成所有组装后进行全面测试, 确保 AICat 所有功能正常.

### 检查清单 / Checklist

#### 机械检查 / Mechanical Check
- [ ] 所有螺丝紧固 / All screws tightened
- [ ] 舵机活动无阻碍 / Servos move freely
- [ ] 屏幕可见且方向正确 / Screen visible and correct orientation
- [ ] 外壳合缝良好 / Shell fits properly

#### 电气检查 / Electrical Check
- [ ] 用万用表测量: VCC 与 GND 无短路 / No short between VCC and GND
- [ ] 所有 GND 连通 / All GND pins connected
- [ ] 外部电源电压正确 (5V 或 6V) / External power voltage correct
- [ ] USB 线连接正常 / USB cable connected

#### 功能测试 / Functional Test

| 测试项 / Test | 通过标准 / Pass Criteria |
|---|---|
| LED 闪烁 / LED Blink | XIAO 内置 LED 闪烁 |
| 串口通信 / Serial | 串口监视器输出正常 (115200) |
| 屏幕显示 / Display | 显示猫咪表情, 颜色正确 |
| I2C 通信 / I2C | PCA9685 响应 (0x40) |
| 耳朵运动 / Ear Movement | 左右耳朵独立转动 |
| 嘴巴运动 / Mouth Movement | 张嘴闭嘴正常 |
| 尾巴运动 / Tail Movement | 尾巴左右摆动 |
| 腿部运动 / Leg Movement | 四条腿独立控制 |
| 音频播放 / Audio | 喇叭发出清晰声音 |
| 摄像头 / Camera | 拍摄图像正常 |
| Wi-Fi / WiFi | 连接到指定 Wi-Fi 网络 |

### 首次完整运行 / First Full Run

```cpp
// 综合测试: 所有功能同时运行
void setup() {
  Serial.begin(115200);
  initDisplay();
  initServos();
  initAudio();
  initCamera();

  Serial.println("=== AICat System Ready ===");
  displayCatFace(HAPPY);
  playMeow();
  moveEars(WIGGLE);
}

void loop() {
  // 基础行为循环
  if (isPetted()) {
    displayCatFace(HAPPY);
    playPurr();
    wagTail();
  } else {
    displayCatFace(IDLE);
    idleAnimation();
  }
}
```

### 故障排除 / Troubleshooting

| 问题 / Problem | 解决方案 / Solution |
|---|---|
| 系统随机重启 | 电源容量不足; 增加 1000uF 电容; 确认外部电源 >= 3A |
| 某个功能不工作 | 逐个注释掉其他功能, 定位冲突; 检查引脚是否冲突 |
| 运行一段时间后死机 | 内存泄漏; 检查 PSRAM 是否启用; 监控 heap 使用量 |
| 舵机发热 | 检查舵机是否长时间堵转; 降低舵机保持力矩 |

---

## 维护建议 / Maintenance Tips

| 项目 / Item | 频率 / Frequency | 说明 / Description |
|---|---|---|
| 检查舵机螺丝 | 每周 / Weekly | 舵机运动可能导致螺丝松动 |
| 清洁屏幕 | 按需 / As needed | 用软布擦拭, 避免使用化学溶剂 |
| 检查线缆 | 每月 / Monthly | 确认线缆无断裂, 接触良好 |
| 电池/电源 | 每次使用前 / Before each use | 确认电源电压在正常范围 |
| 3D 打印件 | 按需 / As needed | 检查裂纹或磨损, 必要时重新打印 |
