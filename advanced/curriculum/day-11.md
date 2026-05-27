# Day 11: 全链路集成与Web控制面板 | Full Pipeline Integration & Web Control Panel

> **今日目标:**
> - 打通完整的语音对话流水线（语音→ASR→AI→TTS→播放）
> - 实现表情与 AI 回复同步
> - 开发 Web 控制面板
> - 设计 WebSocket 消息协议
>
> **产出:** 完整的语音交互 + 表情 + 动作 + Web 控制面板

---

## 时间安排 | Schedule

| 时间 | 活动类型 | 内容 |
|------|----------|------|
| 09:00 - 09:15 | 晨间回顾 | Qwen 3.5 Omni 和 TTS 回顾 |
| 09:15 - 10:30 | 知识讲解 | 全链路架构、消息协议设计、Web 面板开发 |
| 10:30 - 10:45 | 课间休息 | |
| 10:45 - 12:00 | 动手实践 | 整合 ASR + AI + TTS 为完整流水线 |
| 12:00 - 13:30 | 午餐休息 | |
| 13:30 - 15:00 | 项目实战 | Web 控制面板开发 |
| 15:00 - 15:15 | 课间休息 | |
| 15:15 - 16:30 | 拓展练习 | 表情同步优化、延迟优化 |
| 16:30 - 17:00 | 总结分享 | 全链路演示 |

---

## 上午: 全链路集成 | Morning: Full Pipeline Integration

### 完整架构 | Complete Architecture

```
全链路数据流:

ESP32 端:
  麦克风 → I2S采集 → WebSocket → Python后端
  ←─────────────── WebSocket ← TTS音频 ← 后端
  屏幕显示表情 ←──── WebSocket ← 表情命令 ← 后端
  舵机动作 ←──────── WebSocket ← 动作命令 ← 后端

Python 后端:
  /ws/audio → ASR(Paraformer) → Qwen 3.5 Omni → TTS(CosyVoice)
  /ws/camera → 转发到浏览器
  /ws/control ←→ 命令交互
```

### WebSocket 消息协议 | Message Protocol

```python
# 控制命令格式 (JSON)
{
    "type": "expression",
    "name": "happy",       # neutral/happy/sad/surprised/angry
    "loop": true
}

{
    "type": "servo",
    "channel": 12,         # PCA9685 通道
    "angle": 60
}

{
    "type": "gait",
    "action": "walk",      # walk/sit/wave/jump
    "steps": 4
}

{
    "type": "audio",
    "data": "<base64>"     # TTS 音频数据
}

{
    "type": "text",
    "content": "喵~ 你好呀！"
}
```

### 整合代码 | Integration Code

```python
# app.py — 完整的 AICat 后端

from fastapi import FastAPI, WebSocket, WebSocketDisconnect
import asyncio, json, os
from dotenv import load_dotenv

load_dotenv()

app = FastAPI()

class AICatServer:
    def __init__(self):
        self.clients = {}  # {type: websocket}
        self.chat_history = []
        self.is_speaking = False

    async def handle_audio(self, ws: WebSocket):
        await ws.accept()
        self.clients["audio"] = ws
        audio_buffer = b""

        try:
            while True:
                data = await ws.receive_bytes()
                audio_buffer += data

                # 检测到一段语音结束（简化：按大小判断）
                if len(audio_buffer) >= 64000:  # ~2秒
                    # 1. ASR 语音识别
                    text = await self.asr(audio_buffer)
                    audio_buffer = b""
                    if not text:
                        continue

                    print(f"[User] {text}")

                    # 2. AI 对话
                    reply = await self.chat(text)

                    # 3. 表情同步
                    expression = self.detect_emotion(reply)
                    await self.send_command("expression", {
                        "name": expression, "loop": False
                    })

                    # 4. TTS 语音合成
                    audio = await self.tts(reply)

                    # 5. 发送音频到 ESP32
                    await self.send_audio(audio)

                    # 6. 发送文字到控制面板
                    await self.send_command("text", {"content": reply})

                    print(f"[AI] {reply}")
        except WebSocketDisconnect:
            self.clients.pop("audio", None)

    def detect_emotion(self, text: str) -> str:
        """简单的情感检测"""
        if any(w in text for w in ["开心", "哈哈", "喵~", "喜欢"]):
            return "happy"
        elif any(w in text for w in ["难过", "伤心", "呜"]):
            return "sad"
        elif any(w in text for w in ["惊讶", "什么", "?!"]):
            return "surprised"
        return "neutral"

    async def send_command(self, cmd_type: str, data: dict):
        msg = json.dumps({"type": cmd_type, **data})
        for ws in self.clients.values():
            try:
                await ws.send_text(msg)
            except:
                pass

server = AICatServer()

@app.websocket("/ws/audio")
async def ws_audio(ws: WebSocket):
    await server.handle_audio(ws)

@app.websocket("/ws/camera")
async def ws_camera(ws: WebSocket):
    await ws.accept()
    server.clients["camera"] = ws
    try:
        while True:
            data = await ws.receive_bytes()
            if "control" in server.clients:
                await server.clients["control"].send_bytes(data)
    except WebSocketDisconnect:
        server.clients.pop("camera", None)

@app.websocket("/ws/control")
async def ws_control(ws: WebSocket):
    await ws.accept()
    server.clients["control"] = ws
    try:
        while True:
            await ws.receive_text()  # 保持连接
    except WebSocketDisconnect:
        server.clients.pop("control", None)
```

---

## 下午: Web 控制面板 | Afternoon: Web Control Panel

### HTML 控制面板 | Control Panel HTML

```html
<!-- 简化的 Web 控制面板 -->
<!DOCTYPE html>
<html>
<head><title>AICat Control Panel</title></head>
<body>
<h1>AICat 控制面板</h1>

<!-- 摄像头画面 -->
<div>
    <h2>摄像头</h2>
    <canvas id="camera" width="320" height="240"></canvas>
</div>

<!-- ASR 转写文字 -->
<div>
    <h2>对话</h2>
    <div id="chat-log"></div>
</div>

<!-- 表情控制 -->
<div>
    <h2>表情</h2>
    <button onclick="sendExpr('neutral')">平静</button>
    <button onclick="sendExpr('happy')">开心</button>
    <button onclick="sendExpr('sad')">难过</button>
    <button onclick="sendExpr('surprised')">惊讶</button>
</div>

<!-- 舵机控制 -->
<div>
    <h2>舵机</h2>
    <label>嘴巴 <input type="range" id="mouth" min="0" max="180"
         oninput="sendServo(12, this.value)"></label><br>
    <label>尾巴 <input type="range" id="tail" min="0" max="180"
         oninput="sendServo(13, this.value)"></label><br>
</div>

<script>
const ws = new WebSocket(`ws://${location.host}/ws/control`);
const camWs = new WebSocket(`ws://${location.host}/ws/camera`);

function sendExpr(name) {
    ws.send(JSON.stringify({type: "expression", name: name}));
}
function sendServo(ch, angle) {
    ws.send(JSON.stringify({type: "servo", channel: ch, angle: +angle}));
}
</script>
</body>
</html>
```

---

## 拓展练习 | Extension Exercises

### 练习1: 延迟优化
测量从说话到 AI 回复播放的总延迟，尝试通过流式 TTS 减少等待时间。

### 练习2: 断线重连
在 ESP32 端实现 WebSocket 断线自动重连机制。

---

## 今日总结 | Day Summary

| 知识点 | 掌握程度 |
|--------|----------|
| 全链路集成 | 能串联 ASR + AI + TTS + 播放 |
| 消息协议 | 设计了清晰的 JSON 命令格式 |
| 表情同步 | AI 回复内容自动匹配表情 |
| Web 控制面板 | 能在浏览器控制所有功能 |
| 多客户端管理 | 理解 camera/audio/control 三通道 |

*最后更新：2026-05-27*
