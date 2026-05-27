# Day 08: 表情动画系统与LittleFS存储 | Facial Expression System & LittleFS Storage

> **今日目标:**
> - 理解 LittleFS 文件系统在 ESP32 上的使用
> - 掌握自定义分区表配置
> - 实现 JPEG 表情帧存储与动画播放
> - 理解嘴型同步原理
>
> **产出:** 屏幕播放流畅的表情动画，支持多种情绪切换

---

## 时间安排 | Schedule

| 时间 | 活动类型 | 内容 |
|------|----------|------|
| 09:00 - 09:15 | 晨间回顾 | 舵机系统与步态回顾 |
| 09:15 - 10:30 | 知识讲解 | LittleFS 原理、分区表、JPEG 解码、动画引擎 |
| 10:30 - 10:45 | 课间休息 | |
| 10:45 - 12:00 | 动手实践 | LittleFS 读写测试、JPEG 显示 |
| 12:00 - 13:30 | 午餐休息 | |
| 13:30 - 15:00 | 项目实战 | 表情动画播放引擎 |
| 15:00 - 15:15 | 课间休息 | |
| 15:15 - 16:30 | 拓展练习 | flash_files 工具使用、表情素材制作 |
| 16:30 - 17:00 | 总结分享 | 表情效果展示 |

---

## 上午: LittleFS 与分区表 | Morning: LittleFS & Partitions

### ESP32 分区表 | Partition Table

ESP32 的 Flash 被划分为多个区域。AICat 需要较大的文件系统空间来存储表情帧。

```
默认分区 vs 自定义分区:

默认 (4MB Flash):
  # Name     Type   SubType  Offset     Size
  nvs        data   nvs      0x9000     0x4000
  phy_init   data   phy      0xf000     0x1000
  factory    app    factory  0x10000    3M
                    ← 文件系统空间不够!

自定义 (8MB Flash):
  # Name     Type   SubType  Offset     Size
  nvs        data   nvs      0x9000     0x4000
  phy_init   data   phy      0xf000     0x1000
  factory    app    factory  0x10000    2M
  storage    data   spiffs   0x210000   5.9M
                    ← 足够存储表情帧!
```

### LittleFS 基础操作 | LittleFS Basics

```cpp
#include <LittleFS.h>

void setupLittleFS() {
    if (!LittleFS.begin(true)) {  // true = 格式化如果挂载失败
        Serial.println("LittleFS mount failed");
        return;
    }
    Serial.printf("LittleFS: %d bytes free\n",
                  LittleFS.totalBytes() - LittleFS.usedBytes());
}

// 列出所有文件
void listFiles() {
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    while (file) {
        Serial.printf("  %s (%d bytes)\n",
                      file.name(), file.size());
        file = root.openNextFile();
    }
}

// 读取文件
void readFile(const char* path) {
    File file = LittleFS.open(path, "r");
    if (!file) return;
    uint8_t buf[256];
    while (file.available()) {
        int len = file.read(buf, sizeof(buf));
        // 处理数据...
    }
    file.close();
}
```

### JPEG 解码与显示 | JPEG Decode & Display

```cpp
#include <JPEGDEC.h>

JPEGDEC jpeg;

int drawJPEG(JPEGDRAW *draw) {
    // draw->x, draw->y: 绘制位置
    // draw->pPixels: 像素数据 (RGB565)
    // draw->iWidth, draw->iHeight: 尺寸
    tft.drawRGBBitmap(draw->x, draw->y,
                       draw->pPixels,
                       draw->iWidth, draw->iHeight);
    return 1;  // 继续
}

bool displayJPEG(const char* path) {
    File file = LittleFS.open(path, "r");
    if (!file) return false;

    bool ok = jpeg.open((void*)&file,
                        JPEG_FILE_READ_CALLBACK,
                        drawJPEG,
                        true /* scale */);
    if (ok) {
        jpeg.decode(0, 0, 0);
        jpeg.close();
    }
    file.close();
    return ok;
}
```

---

## 下午: 表情动画引擎 | Afternoon: Animation Engine

### 动画帧结构 | Animation Frame Structure

```
LittleFS 中的表情文件:

/expressions/
├── neutral/
│   ├── 000.jpg
│   ├── 001.jpg
│   └── ...
├── happy/
│   ├── 000.jpg
│   ├── 001.jpg
│   └── ...
├── sad/
│   └── ...
└── surprised/
    └── ...

/mouth/
├── closed/
│   └── 000.jpg
├── small_open/
│   └── 000.jpg
├── big_open/
│   └── 000.jpg
└── wide/
    └── 000.jpg
```

### 动画播放引擎 | Animation Playback Engine

```cpp
#define MAX_FRAMES 30
#define ANIM_FPS 15

struct Animation {
    char path[64];       // 表情路径前缀
    int frameCount;      // 帧数
    int currentFrame;    // 当前帧
    unsigned long lastFrameTime;
    bool loop;           // 是否循环播放
};

Animation currentAnim;

void playAnimation(const char* name, bool loop) {
    snprintf(currentAnim.path, sizeof(currentAnim.path),
             "/expressions/%s", name);
    currentAnim.frameCount = countFrames(currentAnim.path);
    currentAnim.currentFrame = 0;
    currentAnim.lastFrameTime = 0;
    currentAnim.loop = loop;
}

int countFrames(const char* path) {
    int count = 0;
    char filepath[80];
    do {
        snprintf(filepath, sizeof(filepath),
                 "%s/%03d.jpg", path, count);
        if (LittleFS.exists(filepath)) count++;
        else break;
    } while (count < MAX_FRAMES);
    return count;
}

void updateAnimation() {
    if (currentAnim.frameCount == 0) return;

    unsigned long now = millis();
    if (now - currentAnim.lastFrameTime < 1000 / ANIM_FPS) return;
    currentAnim.lastFrameTime = now;

    char filepath[80];
    snprintf(filepath, sizeof(filepath),
             "%s/%03d.jpg", currentAnim.path,
             currentAnim.currentFrame);
    displayJPEG(filepath);

    currentAnim.currentFrame++;
    if (currentAnim.currentFrame >= currentAnim.frameCount) {
        if (currentAnim.loop) {
            currentAnim.currentFrame = 0;
        } else {
            currentAnim.frameCount = 0;  // 播放结束
        }
    }
}
```

---

## 拓展练习 | Extension Exercises

### 练习1: flash_files 工具
使用项目中的 `flash_files.ino` 工具将自制表情帧写入 LittleFS：
1. 准备 JPEG 帧（170x320 或更小）
2. 用 `generate_all_headers.py` 生成头文件
3. 上传 `flash_files.ino`，串口输入 `F` 格式化、`W` 写入

### 练习2: 嘴型同步
根据播放的音频文本，在说话时同步切换嘴型帧（closed → small_open → big_open → ...）。

---

## 今日总结 | Day Summary

| 知识点 | 掌握程度 |
|--------|----------|
| 分区表 | 理解 Flash 分区和文件系统空间 |
| LittleFS | 能读写文件、管理存储 |
| JPEG 解码 | 能从文件系统读取并显示 JPEG |
| 动画引擎 | 能播放循环/单次动画 |
| 表情系统 | 理解帧存储和播放流程 |

*最后更新：2026-05-27*
