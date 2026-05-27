# Day 05: 摄像头流媒体与WebSocket通信 | Camera Streaming & WebSocket Communication

> **今日目标:**
> - 掌握XIAO ESP32S3 Sense摄像头的初始化与JPEG图像捕获
> - 理解WebSocket协议的原理与优势
> - 使用ArduinoWebsockets库实现ESP32与PC的实时通信
> - 完成摄像头画面通过Wi-Fi实时传输到PC浏览器
>
> **产出:** 浏览器中实时显示XIAO摄像头捕获的画面

---

## 时间安排 | Schedule

| 时间 | 活动类型 | 内容 |
|------|----------|------|
| 09:00 - 09:15 | 晨间回顾 | 回顾昨日作业，解答疑问 |
| 09:15 - 10:30 | 知识讲解 | 摄像头驱动、JPEG编码、WebSocket协议、Wi-Fi配置 |
| 10:30 - 10:45 | 课间休息 | |
| 10:45 - 12:00 | 动手实践 | 摄像头初始化与JPEG图像捕获 |
| 12:00 - 13:30 | 午餐休息 | |
| 13:30 - 15:00 | 项目实战 | Wi-Fi配置与WebSocket视频流传输 |
| 15:00 - 15:15 | 课间休息 | |
| 15:15 - 16:30 | 拓展练习 | 浏览器端HTML接收与帧率控制 |
| 16:30 - 17:00 | 总结分享 | 今日收获总结 |

## 上午: 摄像头驱动与图像捕获 | Morning: Camera Driver & Image Capture

### 1. XIAO ESP32S3 Sense摄像头 | XIAO ESP32S3 Sense Camera

XIAO ESP32S3 Sense板载OV2640摄像头，通过专用DVP接口连接：

The XIAO ESP32S3 Sense has an onboard OV2640 camera connected via a dedicated DVP interface:

```
OV2640 摄像头规格 Camera specs:
  传感器 Sensor:      1/4" CMOS
  有效像素 Pixels:    200万 (1600x1200)
  输出格式 Output:    YUV422, YCbCr422, RGB565, JPEG
  最大帧率 FPS:       UXGA 15fps, SVGA 30fps, QVGA 60fps
  接口 Interface:     DVP (Digital Video Port)

本项目使用JPEG格式输出以降低传输带宽。
Using JPEG output to reduce bandwidth for transmission.
```

### 2. 摄像头引脚定义与初始化 | Camera Pin Definitions & Initialization

```cpp
// Day 05 - 摄像头初始化
#include "esp_camera.h"

// XIAO ESP32S3 Sense 摄像头引脚定义
// Camera pin definitions for XIAO ESP32S3 Sense
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     15
#define SIOD_GPIO_NUM     4
#define SIOC_GPIO_NUM     5
#define Y9_GPIO_NUM       16
#define Y8_GPIO_NUM       17
#define Y7_GPIO_NUM       18
#define Y6_GPIO_NUM       12
#define Y5_GPIO_NUM       10
#define Y4_GPIO_NUM       8
#define Y3_GPIO_NUM       9
#define Y2_GPIO_NUM       11
#define VSYNC_GPIO_NUM    6
#define HREF_GPIO_NUM     7
#define PCLK_GPIO_NUM     13

bool cameraInit() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;          // 20MHz XCLK
  config.pixel_format = PIXFORMAT_JPEG;     // JPEG输出

  // PSRAM对于高分辨率JPEG至关重要
  // PSRAM is critical for high-resolution JPEG
  if (psramFound()) {
    config.frame_size = FRAMESIZE_QVGA;     // 320x240
    config.jpeg_quality = 10;                // 0-63, 数字越小质量越高
    config.fb_count = 2;                     // 双帧缓冲
    config.grab_mode = CAMERA_GRAB_LATEST;   // 获取最新帧
    Serial.println("PSRAM found: QVGA + dual buffer");
  } else {
    config.frame_size = FRAMESIZE_QQVGA;    // 160x120
    config.jpeg_quality = 12;
    config.fb_count = 1;
    Serial.println("No PSRAM: QQVGA + single buffer");
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    return false;
  }

  // 调整摄像头传感器参数 Adjust sensor parameters
  sensor_t *s = esp_camera_sensor_get();
  s->set_brightness(s, 0);     // -2 to 2
  s->set_contrast(s, 0);       // -2 to 2
  s->set_saturation(s, 0);     // -2 to 2
  s->set_vflip(s, false);      // 垂直翻转 Vertical flip
  s->set_hmirror(s, false);    // 水平镜像 Horizontal mirror

  Serial.println("Camera initialized successfully!");
  return true;
}
```

### 3. 捕获JPEG图像与帧率测试 | Capture JPEG & FPS Test

```cpp
// 捕获一帧 JPEG 图像 Capture a JPEG frame
camera_fb_t* captureFrame() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed!");
    return NULL;
  }
  Serial.printf("Frame: %dx%d, %d bytes\n",
                fb->width, fb->height, fb->len);
  return fb;
}

// 释放帧缓冲（必须调用！）Release frame buffer (must call!)
void releaseFrame(camera_fb_t *fb) {
  esp_camera_fb_return(fb);
}

// 测试帧率 Test FPS
void testFrameRate() {
  Serial.println("Measuring FPS...");
  unsigned long start = millis();
  int count = 0;

  for (int i = 0; i < 50; i++) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (fb) {
      count++;
      esp_camera_fb_return(fb);
    }
  }

  float fps = (float)count * 1000 / (millis() - start);
  Serial.printf("FPS: %.1f (%d frames in %lu ms)\n",
                fps, count, millis() - start);
}
```

## 下午: Wi-Fi与WebSocket视频流 | Afternoon: Wi-Fi & WebSocket Video Streaming

### 4. WebSocket协议 | WebSocket Protocol

```
HTTP vs WebSocket 对比:

HTTP (传统 Traditional):
  客户端 → 请求 → 服务器
  客户端 ← 响应 ← 服务器
  每次请求都要重新建立连接 Re-establish connection each time

WebSocket:
  客户端 ←→ 持久连接 ←→ 服务器
  一次握手，持续双向通信 One handshake, continuous bidirectional

优势 Advantages:
  - 低延迟 Low latency（无需重复握手）
  - 双向通信 Bidirectional
  - 减少协议开销 Less protocol overhead
  - 适合实时数据 Ideal for real-time data
```

### 5. Wi-Fi配置与连接 | Wi-Fi Configuration

```cpp
#include <WiFi.h>
#include <ArduinoWebsockets.h>

using namespace websockets;

// ===== 修改为你的网络配置 Modify for your network =====
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Python后端地址 Python backend address
const char* ws_server = "ws://192.168.1.100:8081/ws/camera";

WebsocketsClient client;

bool connectWiFi() {
  Serial.printf("Connecting to WiFi: %s\n", ssid);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nWiFi connected! IP: %s\n",
                  WiFi.localIP().toString().c_str());
    // 打印信号强度 Print signal strength
    Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
    return true;
  }
  Serial.println("\nWiFi connection failed!");
  return false;
}
```

### 6. WebSocket视频流发送 | WebSocket Video Streaming

```cpp
// 通过 WebSocket 发送视频帧 Stream video via WebSocket
void streamVideo() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) return;

  if (client.available()) {
    // 发送JPEG二进制数据 Send JPEG binary data
    client.sendBinary((const char*)fb->buf, fb->len);
  } else {
    // 断线重连 Reconnect on disconnect
    Serial.println("WS disconnected, reconnecting...");
    client.connect(ws_server);
  }

  esp_camera_fb_return(fb);
}

// 完整的 setup 和 loop Complete setup and loop
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== Camera Streaming System ===");

  if (!connectWiFi()) {
    Serial.println("Cannot proceed without WiFi!");
    while (1) delay(1000);
  }

  if (!cameraInit()) {
    Serial.println("Camera init failed!");
    while (1) delay(1000);
  }

  testFrameRate();  // 测试本地帧率

  // 连接WebSocket服务器 Connect to WebSocket server
  Serial.printf("Connecting to: %s\n", ws_server);
  if (client.connect(ws_server)) {
    Serial.println("WebSocket connected!");
  } else {
    Serial.println("WebSocket connection failed!");
  }
}

void loop() {
  static unsigned long lastFrame = 0;
  int fps = 15;  // 目标帧率 Target FPS
  unsigned long interval = 1000 / fps;

  if (millis() - lastFrame >= interval) {
    streamVideo();
    lastFrame = millis();
  }

  // 处理WebSocket事件 Process WebSocket events
  client.poll();
}
```

### 7. 浏览器端接收页面 | Browser-side HTML Viewer

在PC上创建HTML页面接收并显示视频流：

Create an HTML page on PC to receive and display the video stream:

```html
<!-- camera_viewer.html - 在浏览器中打开 Open in browser -->
<!DOCTYPE html>
<html>
<head>
    <title>Desktop Pet Camera</title>
    <style>
        body { background: #222; color: #fff; font-family: Arial; text-align: center; }
        #video { border: 2px solid #444; border-radius: 8px; max-width: 640px; margin: 10px; }
        #status { font-size: 14px; padding: 10px; }
        #fps { font-size: 12px; color: #888; }
    </style>
</head>
<body>
    <h1>AICat Camera Viewer</h1>
    <div id="status">Connecting...</div>
    <img id="video" />
    <div id="fps"></div>

    <script>
        const img = document.getElementById('video');
        const statusDiv = document.getElementById('status');
        const fpsDiv = document.getElementById('fps');
        let frameCount = 0;
        let lastTime = performance.now();

        // 修改为你的后端地址 Modify to your backend address
        const ws = new WebSocket('ws://192.168.1.100:8081/ws/camera');
        ws.binaryType = 'arraybuffer';

        ws.onopen = () => {
            statusDiv.textContent = 'Connected! Streaming...';
            statusDiv.style.color = '#0f0';
        };

        ws.onmessage = (event) => {
            // 接收JPEG二进制数据并显示 Receive and display JPEG
            const blob = new Blob([event.data], { type: 'image/jpeg' });
            URL.revokeObjectURL(img.src);  // 释放旧的Blob Release old blob
            img.src = URL.createObjectURL(blob);

            // 计算FPS Calculate FPS
            frameCount++;
            const now = performance.now();
            if (now - lastTime >= 1000) {
                fpsDiv.textContent = `FPS: ${frameCount}`;
                frameCount = 0;
                lastTime = now;
            }
        };

        ws.onclose = () => {
            statusDiv.textContent = 'Disconnected. Reconnecting in 3s...';
            statusDiv.style.color = '#f00';
            setTimeout(() => location.reload(), 3000);
        };

        ws.onerror = () => {
            statusDiv.textContent = 'Connection error!';
            statusDiv.style.color = '#f00';
        };
    </script>
</body>
</html>
```

## 拓展练习 | Extension Exercises

### 练习1: 帧率动态调节 | Dynamic FPS Control

通过串口命令动态调整帧率：

```cpp
int targetFPS = 15;

void checkSerialCommands() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.startsWith("fps ")) {
      targetFPS = cmd.substring(4).toInt();
      targetFPS = constrain(targetFPS, 1, 30);
      Serial.printf("FPS set to %d\n", targetFPS);
    } else if (cmd == "quality ") {
      sensor_t *s = esp_camera_sensor_get();
      int q = cmd.substring(8).toInt();
      s->set_quality(s, constrain(q, 5, 40));
      Serial.printf("Quality set to %d\n", q);
    }
  }
}
```

### 练习2: 图像质量对比 | Image Quality Comparison
分别使用 quality 5/10/20/40 拍照，比较文件大小和画面质量。

### 练习3: 双向通信 | Bidirectional Communication
从浏览器发送命令到ESP32（如拍照保存、切换分辨率）。

## 今日总结 | Day Summary

| 知识点 | 掌握程度 |
|--------|----------|
| OV2640摄像头 | 能初始化摄像头并配置参数（分辨率、质量、帧缓冲） |
| JPEG输出 | 理解JPEG编码优势和PSRAM对帧缓冲的作用 |
| Wi-Fi连接 | 能配置Wi-Fi并获取IP地址、检测信号强度 |
| WebSocket协议 | 理解全双工通信优势，能发送二进制数据 |
| 视频流传输 | 能通过WebSocket持续发送JPEG帧到PC |
| 浏览器显示 | 能创建HTML页面接收Blob并实时显示 |

- **明日预告**: 麦克风录音与语音处理
