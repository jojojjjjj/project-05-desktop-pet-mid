# Day 10: Qwen 3.5 Omni多模态大模型集成 | Qwen 3.5 Omni Multimodal AI Integration

> **今日目标:**
> - 了解 Qwen 3.5 Omni 多模态大模型能力
> - 调用 DashScope API 进行文本/语音对话
> - 实现流式响应处理
> - 集成 TTS（文本转语音）功能
>
> **产出:** 能通过语音与 AI 对话，并获得语音回复

---

## 时间安排 | Schedule

| 时间 | 活动类型 | 内容 |
|------|----------|------|
| 09:00 - 09:15 | 晨间回顾 | Python 后端与 ASR 回顾 |
| 09:15 - 10:30 | 知识讲解 | 多模态大模型原理、DashScope API、流式响应 |
| 10:30 - 10:45 | 课间休息 | |
| 10:45 - 12:00 | 动手实践 | API 调用测试、文本对话 |
| 12:00 - 13:30 | 午餐休息 | |
| 13:30 - 15:00 | 项目实战 | TTS 语音合成 + 流式音频发送 |
| 15:00 - 15:15 | 课间休息 | |
| 15:15 - 16:30 | 拓展练习 | 多轮对话上下文、角色设定 |
| 16:30 - 17:00 | 总结分享 | AI 集成效果展示 |

---

## 上午: Qwen 3.5 Omni 模型 | Morning: Qwen 3.5 Omni

### 什么是多模态大模型？| What is a Multimodal LLM?

```
传统 LLM (纯文本):
  用户输入文字 → AI → 输出文字

多模态 LLM (文本+语音+图像):
  用户输入语音 ─┐
  用户输入文字 ─┤→ AI → 输出文字 + 语音
  用户输入图像 ─┘

Qwen 3.5 Omni 特点:
  - 支持文本和语音同时输入
  - 能理解语音中的情感和语气
  - 流式输出，延迟低
  - 支持中文和英文
```

### DashScope API 调用 | API Call

```python
from dashscope import Generation

async def chat_with_ai(user_text: str, history: list) -> str:
    """与 Qwen 3.5 Omni 对话"""
    messages = [
        {"role": "system", "content": "你是一只可爱的桌面猫咪，名叫小喵。"
         "你性格活泼、调皮、有点傲娇。回答要简短有趣，"
         "像猫咪一样说话，偶尔加'喵~'。"},
    ]
    messages.extend(history)
    messages.append({"role": "user", "content": user_text})

    response = Generation.call(
        model="qwen3.5-omni",
        messages=messages,
        stream=True,  # 流式输出
    )

    full_reply = ""
    for chunk in response:
        if chunk.status_code == 200:
            content = chunk.output.choices[0].message.content
            full_reply += content
            print(content, end="", flush=True)
        else:
            print(f"\nAPI error: {chunk.message}")
            break

    print()  # 换行
    return full_reply
```

### API 费用说明 | API Cost

| 项目 | 免费额度 | 价格 |
|------|----------|------|
| 新用户 | 100万 tokens | — |
| Qwen 3.5 Omni 输入 | — | ¥0.002/千tokens |
| Qwen 3.5 Omni 输出 | — | ¥0.006/千tokens |

一次对话大约消耗 500-2000 tokens，费用约 ¥0.003-0.01。

---

## 下午: TTS 语音合成 | Afternoon: TTS

### 文本转语音 | Text to Speech

```python
from dashscope.audio.tts_v2 import SpeechSynthesizer

async def text_to_speech(text: str) -> bytes:
    """将文本转换为语音"""
    synthesizer = SpeechSynthesizer(
        model="cosyvoice-v2",
        voice="longxiaochun",  # 语音角色
        format="pcm",
        sample_rate=16000,
    )

    audio_data = b""
    result = synthesizer.call(text)
    if result.status_code == 200:
        audio_data = result.get_audio_data()
    else:
        print(f"TTS error: {result.message}")

    return audio_data

async def send_audio_to_esp32(audio_data: bytes):
    """通过 WebSocket 发送音频到 ESP32"""
    if audio_ws and len(audio_data) > 0:
        # 分块发送，避免 WebSocket 帧过大
        CHUNK_SIZE = 4096
        for i in range(0, len(audio_data), CHUNK_SIZE):
            chunk = audio_data[i:i+CHUNK_SIZE]
            await audio_ws.send_bytes(chunk)
            await asyncio.sleep(0.05)  # 模拟流式播放
```

### 完整对话流水线 | Full Dialogue Pipeline

```python
async def handle_dialogue(user_text: str):
    """完整的对话流程: 文本 → AI → TTS → 播放"""
    # 1. 获取 AI 回复
    reply = await chat_with_ai(user_text, chat_history)

    # 2. 生成语音
    audio = await text_to_speech(reply)

    # 3. 发送到 ESP32 播放
    await send_audio_to_esp32(audio)

    # 4. 更新聊天历史
    chat_history.append({"role": "user", "content": user_text})
    chat_history.append({"role": "assistant", "content": reply})

    # 5. 发送表情命令
    if "开心" in reply or "喵~" in reply:
        await send_command("expression:happy")
    elif "难过" in reply:
        await send_command("expression:sad")
```

---

## 拓展练习 | Extension Exercises

### 练习1: 角色设定
修改 system prompt，让猫咪有不同性格（高冷猫、话痨猫、学霸猫）。

### 练习2: 多轮对话
维护一个聊天历史列表（最近 10 轮），让对话有上下文连贯性。

---

## 今日总结 | Day Summary

| 知识点 | 掌握程度 |
|--------|----------|
| 多模态大模型 | 理解 Qwen 3.5 Omni 的能力 |
| DashScope API | 能调用文本生成和流式响应 |
| TTS 语音合成 | 能将文本转为音频 |
| 对话流水线 | 理解 ASR → AI → TTS → 播放流程 |
| API 成本 | 了解用量和费用控制 |

*最后更新：2026-05-27*
