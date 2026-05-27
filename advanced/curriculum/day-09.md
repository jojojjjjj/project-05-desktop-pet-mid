# Day 09: Python后端搭建与语音识别 | Python Backend Setup & Speech Recognition

> **今日目标:**
> - 搭建 Python 后端服务器（FastAPI + WebSocket）
> - 实现 ASR（语音识别）功能
> - 配置 DashScope API 密钥
> - 理解前后端通信架构
>
> **产出:** Python 后端能接收音频并通过 ASR 转文字

---

## 时间安排 | Schedule

| 时间 | 活动类型 | 内容 |
|------|----------|------|
| 09:00 - 09:15 | 晨间回顾 | 表情系统回顾、LittleFS 使用经验 |
| 09:15 - 10:30 | 知识讲解 | Python 后端架构、WebSocket 服务端、ASR 原理 |
| 10:30 - 10:45 | 课间休息 | |
| 10:45 - 12:00 | 动手实践 | FastAPI 服务搭建 + WebSocket 端点 |
| 12:00 - 13:30 | 午餐休息 | |
| 13:30 - 15:00 | 项目实战 | ASR 集成 + 音频接收 |
| 15:00 - 15:15 | 课间休息 | |
| 15:15 - 16:30 | 拓展练习 | 浏览器测试页面、多客户端管理 |
| 16:30 - 17:00 | 总结分享 | 后端架构理解检验 |

---

## 上午: Python 后端架构 | Morning: Python Backend

### 环境搭建 | Environment Setup

```bash
# 创建虚拟环境
cd upload_facial_expression/integrated/server
python3 -m venv .venv

# 激活虚拟环境
# Windows:
.venv\Scripts\Activate.ps1
# macOS/Linux:
source .venv/bin/activate

# 安装依赖
pip install -r requirements.txt
```

### .env 配置 | Configuration

```bash
# .env 文件
DASHSCOPE_API_KEY=sk-your-api-key-here
ASR_DEBUG_RAW=0
```

### FastAPI + WebSocket 服务端 | Server

```python
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.responses import HTMLResponse
import asyncio
import os
from dotenv import load_dotenv

load_dotenv()

app = FastAPI()

# WebSocket 连接管理
class ConnectionManager:
    def __init__(self):
        self.camera_ws = None
        self.audio_ws = None
        self.control_ws = None

    async def handle_camera(self, ws: WebSocket):
        await ws.accept()
        self.camera_ws = ws
        print("[Camera] Client connected")
        try:
            while True:
                data = await ws.receive_bytes()
                # 转发摄像头帧到浏览器
                if self.control_ws:
                    await self.control_ws.send_bytes(data)
        except WebSocketDisconnect:
            self.camera_ws = None

    async def handle_audio(self, ws: WebSocket):
        await ws.accept()
        self.audio_ws = ws
        print("[Audio] Client connected")
        audio_buffer = b""
        try:
            while True:
                data = await ws.receive_bytes()
                audio_buffer += data
                # 收集足够音频后进行 ASR
                if len(audio_buffer) >= 32000:  # ~1秒 @16kHz 16bit
                    text = await recognize_speech(audio_buffer)
                    audio_buffer = b""
                    if text:
                        print(f"[ASR] {text}")
        except WebSocketDisconnect:
            self.audio_ws = None

manager = ConnectionManager()

@app.websocket("/ws/camera")
async def ws_camera(ws: WebSocket):
    await manager.handle_camera(ws)

@app.websocket("/ws/audio")
async def ws_audio(ws: WebSocket):
    await manager.handle_audio(ws)

@app.get("/")
async def index():
    return HTMLResponse("<h1>AICat Server</h1>")

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8081)
```

---

## 下午: ASR 语音识别 | Afternoon: ASR

### DashScope ASR 集成 | DashScope ASR

```python
import dashscope
from dashscope.audio.asr import Recognition

dashscope.api_key = os.getenv("DASHSCOPE_API_KEY")

async def recognize_speech(audio_data: bytes) -> str:
    """使用 DashScope Paraformer 进行语音识别"""
    try:
        result = Recognition.call(
            model="paraformer-realtime-v2",
            audio=audio_data,
            format="pcm",
            sample_rate=16000,
        )
        if result.status_code == 200:
            return result.output.get("text", "")
        else:
            print(f"ASR error: {result.message}")
            return ""
    except Exception as e:
        print(f"ASR exception: {e}")
        return ""
```

### requirements.txt

```
fastapi>=0.104.0
uvicorn>=0.24.0
websockets>=12.0
python-dotenv>=1.0.0
dashscope>=1.14.0
numpy>=1.24.0
```

### 启动与测试 | Start & Test

```bash
# 启动后端
python app.py

# 浏览器访问
# http://localhost:8081

# ESP32 连接到
# ws://电脑IP:8081/ws/camera
# ws://电脑IP:8081/ws/audio
```

---

## 拓展练习 | Extension Exercises

### 练习1: Web 测试页面
在 FastAPI 中添加一个简单的 HTML 页面，显示摄像头画面和 ASR 转写文字。

### 练习2: 音频调试
保存接收到的音频为 WAV 文件，用电脑播放验证音质。

---

## 今日总结 | Day Summary

| 知识点 | 掌握程度 |
|--------|----------|
| Python venv | 能创建和管理虚拟环境 |
| FastAPI | 能创建 WebSocket 服务端 |
| DashScope API | 理解 API Key 配置和调用方式 |
| ASR 语音识别 | 能将 PCM 音频转为文字 |
| 前后端架构 | 理解 ESP32 ↔ Python 的通信流程 |

*最后更新：2026-05-27*
