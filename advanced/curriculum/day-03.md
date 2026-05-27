# Day 03: ST7789屏幕驱动与图形显示 | ST7789 Display Driver & Graphics

> **今日目标:**
> - 理解SPI通信协议及其在屏幕驱动中的应用
> - 完成ST7789屏幕的初始化和基本图形绘制
> - 掌握Adafruit GFX库的核心绘图函数
> - 设计并绘制桌面宠物的第一个面部表情
>
> **产出:** 在ST7789屏幕上显示一个由程序绘制的猫咪面部表情

---

## 时间安排 | Schedule

| 时间 | 活动类型 | 内容 |
|------|----------|------|
| 09:00 - 09:15 | 晨间回顾 | 回顾昨日作业，解答疑问 |
| 09:15 - 10:30 | 知识讲解 | SPI协议与ST7789驱动原理 |
| 10:30 - 10:45 | 课间休息 | |
| 10:45 - 12:00 | 动手实践 | 屏幕接线与基本图形绘制 |
| 12:00 - 13:30 | 午餐休息 | |
| 13:30 - 15:00 | 项目实战 | 绘制猫咪面部表情 |
| 15:00 - 15:15 | 课间休息 | |
| 15:15 - 16:30 | 拓展练习 | 多表情切换与眨眼动画 |
| 16:30 - 17:00 | 总结分享 | 今日收获总结 |

## 上午: SPI协议与屏幕驱动 | Morning: SPI Protocol & Display Driver

### 1. SPI通信协议 | SPI Communication Protocol

SPI（Serial Peripheral Interface）是一种高速同步串行通信协议，比I2C快得多，适合驱动显示屏：

SPI (Serial Peripheral Interface) is a high-speed synchronous serial protocol, much faster than I2C, ideal for display driving:

```
SPI四线制通信:
  MOSI (Master Out Slave In)  → 主机发送数据 Master sends data
  MISO (Master In Slave Out)  → 主机接收数据（屏幕不需要 Screen doesn't need）
  SCK  (Serial Clock)         → 时钟信号 Clock signal
  CS   (Chip Select)          → 片选（低电平有效 Active low）

ST7789额外控制线 Additional control lines:
  DC   (Data/Command)         → 数据/命令选择 (0=命令, 1=数据)
  RST  (Reset)                → 复位（低电平有效 Active low）
```

XIAO ESP32S3与ST7789的接线 | Wiring between XIAO ESP32S3 and ST7789:

| ST7789引脚 | XIAO引脚 | 说明 Description |
|-----------|---------|----------------|
| GND | GND | 地线 Ground |
| VCC | 3.3V | 电源（注意不要接5V！）Power (do NOT use 5V!) |
| SCL (SCK) | D10 | SPI时钟 SPI clock |
| SDA (MOSI) | D8 | SPI数据 SPI data |
| RES (RST) | D1 | 复位 Reset |
| DC | D0 | 数据/命令 Data/Command |
| CS | D4 | 片选 Chip Select |

### 2. ST7789屏幕规格 | ST7789 Display Specs

```
ST7789 1.47" TFT LCD:
  分辨率 Resolution:   172 x 320 像素
  色深 Color depth:    16位 RGB565 (65536色)
  接口 Interface:      SPI (最高时钟频率约 Max clock ~26MHz)
  像素格式 Pixel format: RGB565
    红色 R: 5位 (bit 15-11)
    绿色 G: 6位 (bit 10-5)
    蓝色 B: 5位 (bit 4-0)

  颜色示例 Color examples:
    黑色 Black:   0x0000
    白色 White:   0xFFFF
    红色 Red:     0xF800
    绿色 Green:   0x07E0
    蓝色 Blue:    0x001F
    黄色 Yellow:  0xFFE0
    橙色 Orange:  0xFD20
```

### 3. 屏幕初始化 | Screen Initialization

```cpp
// Day 03 - ST7789 屏幕初始化
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// 引脚定义 Pin definitions
#define TFT_CS    D4
#define TFT_DC    D0
#define TFT_RST   D1

// 创建屏幕对象 Create display object
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// 屏幕尺寸 Screen dimensions
#define SCREEN_W 172
#define SCREEN_H 320

// 颜色定义 (RGB565)
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0
#define COLOR_ORANGE  0xFD20
#define COLOR_PINK    0xF81F
#define COLOR_CYAN    0x07FF

// 自定义颜色辅助函数 Custom color helper
uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== ST7789 Screen Test ===");

  // 初始化SPI
  SPI.begin(D10, -1, D8, TFT_CS);  // SCK, MISO, MOSI, SS

  // 初始化ST7789屏幕
  tft.init(SCREEN_W, SCREEN_H);
  tft.setSPISpeed(40000000);        // SPI速度40MHz

  // 清屏 Clear screen
  tft.fillScreen(COLOR_BLACK);

  Serial.printf("Screen initialized: %d x %d\n", SCREEN_W, SCREEN_H);
}

void loop() {}
```

### 4. Adafruit GFX绘图函数 | Adafruit GFX Drawing Functions

```cpp
// === 基本图形绘制 Basic Drawing Primitives ===

// 像素 Pixel
tft.drawPixel(x, y, color);

// 线段 Line
tft.drawLine(x0, y0, x1, y1, color);

// 水平线/垂直线 Fast H/V Line (比drawLine更快 faster than drawLine)
tft.drawFastHLine(x, y, width, color);
tft.drawFastVLine(x, y, height, color);

// 矩形 Rectangle
tft.drawRect(x, y, w, h, color);             // 空心 Outline
tft.fillRect(x, y, w, h, color);              // 实心 Filled
tft.drawRoundRect(x, y, w, h, radius, color); // 圆角 Rounded corners
tft.fillRoundRect(x, y, w, h, radius, color);

// 圆形 Circle
tft.drawCircle(x_center, y_center, radius, color);  // 空心
tft.fillCircle(x_center, y_center, radius, color);   // 实心

// 三角形 Triangle
tft.drawTriangle(x0, y0, x1, y1, x2, y2, color);
tft.fillTriangle(x0, y0, x1, y1, x2, y2, color);

// 文字 Text
tft.setCursor(x, y);                  // 设置位置 Set position
tft.setTextColor(color);              // 文字颜色 Text color
tft.setTextColor(fg, bg);             // 带背景色 With background
tft.setTextSize(size);                // 字号 (1=最小)
tft.setTextWrap(true);                // 自动换行 Auto wrap
tft.print("Hello");                   // 打印 Print
tft.println("World");                 // 打印并换行 Print + newline
```

### 5. 坐标系统 | Coordinate System

```
ST7789 坐标系 Coordinate System:
  (0,0) ──────────────→ X (172)
    │
    │    ┌─────────────┐
    │    │             │
    │    │  屏幕区域   │
    │    │  Screen Area│
    │    │             │
    │    └─────────────┘
    │
    ↓
    Y (320)

  旋转控制 Rotation control:
  tft.setRotation(0);  // 默认方向 Default
  tft.setRotation(1);  // 横向 Landscape
  tft.setRotation(2);  // 旋转180° Upside down
  tft.setRotation(3);  // 横向(反向) Landscape reversed
```

## 下午: 猫咪面部绘制 | Afternoon: Cat Face Drawing

### 6. 绘制猫咪面部 | Draw Cat Face

```cpp
// Day 03 - 猫咪面部表情绘制
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#define TFT_CS    D4
#define TFT_DC    D0
#define TFT_RST   D1

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

#define SCREEN_W 172
#define SCREEN_H 320

// 屏幕中心坐标 Screen center
#define CX (SCREEN_W / 2)  // 86
#define CY 120              // 面部中心Y Face center Y

// 颜色定义 Colors
#define COL_BG      0xFFFF  // 白色背景 White background
#define COL_FACE    0xFD20  // 橙色面部 Orange face
#define COL_EAR     0xFA20  // 深橙色耳朵 Dark orange ears
#define COL_EYE_W   0xFFFF  // 白色眼白 White sclera
#define COL_EYE_B   0x0000  // 黑色瞳孔 Black pupil
#define COL_NOSE    0xF81F  // 粉色鼻子 Pink nose
#define COL_MOUTH   0x0000  // 黑色嘴巴 Black mouth
#define COL_WHISKER 0x0000  // 黑色胡须 Black whiskers

void drawCatNeutral() {
  tft.fillScreen(COL_BG);

  // --- 耳朵 Ears ---
  // 左耳 Left ear (三角形 triangle)
  tft.fillTriangle(
    CX - 40, CY - 30,
    CX - 50, CY - 70,
    CX - 10, CY - 40,
    COL_EAR
  );
  // 右耳 Right ear
  tft.fillTriangle(
    CX + 40, CY - 30,
    CX + 50, CY - 70,
    CX + 10, CY - 40,
    COL_EAR
  );

  // --- 面部 Face (圆形近似 circle) ---
  tft.fillCircle(CX, CY, 45, COL_FACE);

  // --- 眼睛 Eyes ---
  // 左眼眼白 Left eye white
  tft.fillCircle(CX - 18, CY - 5, 14, COL_EYE_W);
  // 左眼瞳孔 Left pupil
  tft.fillCircle(CX - 18, CY - 5, 8, COL_EYE_B);
  // 左眼高光 Left highlight
  tft.fillCircle(CX - 15, CY - 8, 3, COLOR_WHITE);

  // 右眼眼白 Right eye white
  tft.fillCircle(CX + 18, CY - 5, 14, COL_EYE_W);
  // 右眼瞳孔 Right pupil
  tft.fillCircle(CX + 18, CY - 5, 8, COL_EYE_B);
  // 右眼高光 Right highlight
  tft.fillCircle(CX + 21, CY - 8, 3, COLOR_WHITE);

  // --- 鼻子 Nose ---
  tft.fillTriangle(
    CX - 4, CY + 10,
    CX + 4, CY + 10,
    CX,     CY + 15,
    COL_NOSE
  );

  // --- 嘴巴 Mouth ---
  tft.drawLine(CX, CY + 15, CX - 12, CY + 22, COL_MOUTH);
  tft.drawLine(CX, CY + 15, CX + 12, CY + 22, COL_MOUTH);

  // --- 胡须 Whiskers ---
  // 左侧 Left side
  tft.drawLine(CX - 15, CY + 10, CX - 55, CY + 2,  COL_WHISKER);
  tft.drawLine(CX - 15, CY + 13, CX - 55, CY + 13, COL_WHISKER);
  tft.drawLine(CX - 15, CY + 16, CX - 55, CY + 24, COL_WHISKER);
  // 右侧 Right side
  tft.drawLine(CX + 15, CY + 10, CX + 55, CY + 2,  COL_WHISKER);
  tft.drawLine(CX + 15, CY + 13, CX + 55, CY + 13, COL_WHISKER);
  tft.drawLine(CX + 15, CY + 16, CX + 55, CY + 24, COL_WHISKER);
}
```

### 7. 多表情切换 | Expression Switching

```cpp
// 开心表情 - 眯眼 + 大嘴巴
void drawCatHappy() {
  drawCatNeutral();

  // 覆盖眼睛为弯月形 Override eyes with crescents
  tft.fillCircle(CX - 18, CY - 2, 10, COL_EYE_W);
  tft.fillRect(CX - 30, CY - 12, 25, 10, COL_FACE);
  tft.fillCircle(CX + 18, CY - 2, 10, COL_EYE_W);
  tft.fillRect(CX + 6, CY - 12, 25, 10, COL_FACE);

  // 大嘴巴 Wide mouth
  tft.drawLine(CX - 15, CY + 18, CX, CY + 28, COL_MOUTH);
  tft.drawLine(CX, CY + 28, CX + 15, CY + 18, COL_MOUTH);
}

// 困倦表情 - 半闭眼 + 小嘴巴
void drawCatSleepy() {
  drawCatNeutral();

  // 半闭眼 Half-closed eyes
  tft.fillRect(CX - 32, CY - 16, 28, 8, COL_FACE);
  tft.fillRect(CX + 5, CY - 16, 28, 8, COL_FACE);

  // 小嘴巴 Small mouth (直线 line)
  tft.drawLine(CX - 5, CY + 20, CX + 5, CY + 20, COL_MOUTH);
}

// 惊讶表情 - 大圆眼 + O型嘴
void drawCatSurprised() {
  drawCatNeutral();

  // 更大的瞳孔 Bigger pupils
  tft.fillCircle(CX - 18, CY - 5, 12, COL_EYE_B);
  tft.fillCircle(CX + 18, CY - 5, 12, COL_EYE_B);
  tft.fillCircle(CX - 14, CY - 9, 4, COLOR_WHITE);
  tft.fillCircle(CX + 22, CY - 9, 4, COLOR_WHITE);

  // O型嘴巴 O-shaped mouth
  tft.drawCircle(CX, CY + 25, 8, COL_MOUTH);
}

void setup() {
  Serial.begin(115200);
  SPI.begin(D10, -1, D8, TFT_CS);
  tft.init(SCREEN_W, SCREEN_H);
  tft.setSPISpeed(40000000);
  tft.setRotation(2);
}

void loop() {
  Serial.println("Expression: Neutral");
  drawCatNeutral();
  delay(3000);

  Serial.println("Expression: Happy");
  drawCatHappy();
  delay(3000);

  Serial.println("Expression: Sleepy");
  drawCatSleepy();
  delay(3000);

  Serial.println("Expression: Surprised");
  drawCatSurprised();
  delay(3000);
}
```

## 拓展练习 | Extension Exercises

### 练习1: 眨眼动画 | Blink Animation

```cpp
void blinkAnimation() {
  // 闭眼 Close eyes
  for (int i = 0; i <= 8; i++) {
    tft.fillRect(CX - 32, CY - 14, 28, i, COL_FACE);
    tft.fillRect(CX + 5,  CY - 14, 28, i, COL_FACE);
    delay(30);
  }
  delay(50);
  // 睁眼 Open eyes
  for (int i = 8; i >= 0; i--) {
    tft.fillRect(CX - 32, CY - 14, 28, i, COL_FACE);
    tft.fillRect(CX + 5,  CY - 14, 28, i, COL_FACE);
    delay(30);
  }
}
```

### 练习2: 自定义颜色方案 | Custom Color Scheme
设计你自己喜欢的猫咪颜色方案（灰猫、黑猫、蓝猫等）。

### 练习3: 全屏渐变 | Screen Gradient
实现屏幕背景色从上到下的渐变效果。

```cpp
void drawGradient() {
  for (int y = 0; y < SCREEN_H; y++) {
    uint16_t color = tft.color565(
      map(y, 0, SCREEN_H, 255, 0),   // R
      map(y, 0, SCREEN_H, 100, 200),  // G
      map(y, 0, SCREEN_H, 200, 255)   // B
    );
    tft.drawFastHLine(0, y, SCREEN_W, color);
  }
}
```

## 今日总结 | Day Summary

| 知识点 | 掌握程度 |
|--------|----------|
| SPI通信 | 理解四线制和速率优势 |
| ST7789驱动 | 能初始化屏幕并显示图形 |
| Adafruit GFX | 掌握基本绘图函数（圆、三角、线、矩形） |
| RGB565颜色 | 理解16位颜色格式与自定义颜色 |
| 表情设计 | 成功绘制猫咪面部（中性、开心、困倦、惊讶） |
| 坐标系统 | 理解屏幕像素坐标和旋转设置 |

- **明日预告**: 硬件组装与音频系统
