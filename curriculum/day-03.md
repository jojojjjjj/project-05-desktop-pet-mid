# Day 03: PCB焊接与硬件组装(上) | PCB Soldering & Assembly Part 1

> **今日目标:**
> - 完成主板电源电路焊接（DC-DC降压、充电管理）
> - 完成ESP32-S3模组焊接
> - 上电测试：测量各电压测试点，烧录测试固件
> - 完成BMI270传感器焊接
> - 阅读和理解SparkBot原理图
>
> **产出:** 主板电源和核心电路焊接完成，上电测试通过，blink程序成功运行

---

## 时间安排 | Schedule

| 时间 | 活动类型 | 内容 |
|------|----------|------|
| 09:00 - 09:15 | 晨间活动 | 焊接练习板检查，工具准备 |
| 09:15 - 10:30 | 知识讲解 | 原理图阅读、电源电路分析 |
| 10:30 - 10:45 | 课间休息 | |
| 10:45 - 12:00 | 动手实践 | 焊接DC-DC、充电电路、ESP32-S3模组 |
| 12:00 - 13:30 | 午餐休息 | |
| 13:30 - 15:00 | 项目实战 | 上电测试、烧录blink |
| 15:00 - 15:15 | 课间休息 | |
| 15:15 - 16:30 | 拓展练习 | 焊接BMI270、初级调试 |
| 16:30 - 17:00 | 总结分享 | 焊接质量互检、经验交流 |

---

## 上午: 原理图阅读与主板焊接 | Morning: Schematic Reading & Main Board Soldering

### 为什么要学原理图? | Why Learn Schematics?

原理图（Schematic）是电子工程师的"地图"。它用符号表示各个元器件和它们之间的连接关系。学会阅读原理图，你就能理解任何一块电路板是怎么工作的——这比照着图片焊接要靠谱得多。

A schematic is the electronics engineer's "map". It uses symbols to represent components and their connections. Learning to read schematics lets you understand how any circuit board works -- much more reliable than soldering by looking at pictures.

### 任务3.1: 原理图阅读入门 (30分钟)

**原理图基本符号:**

```
常用原理图符号速查:

电源符号:              接地符号:
  VCC  ───┬───           GND ───┬───
          │                      │
          ▼                      ▼
         ─┴─ 3.3V                ═══  (三条横线)

电阻:      电容(无极性):    电容(有极性):
  ──[R]──   ──||──           ──|(──  (曲线端为负极)
  
LED:        二极管:         电感:
  ──▶|──     ──▶|──           ──(((──

芯片/IC:     连接点:          不连接(交叉):
  ┌──────┐       │              ──┼──
  │      │   ────●────           (无点)
  └──────┘
```

**SparkBot主板原理图关键部分阅读:**

```
电源部分 (Power Section):
  
  VBUS(USB 5V) ──┬──[D1 防反接]──┬── TP4054 ──┬── 电池
                  │               │  充电管理   │
                  │               │             │
                  └── SY8089 ─────┴── 3.3V输出 ── ESP32-S3
                      DC-DC降压                    + 所有外设

关键测试点:
  - VBUS: 5V (±0.25V)
  - VCC_3V3: 3.3V (±0.1V)
  - VBAT: 3.7-4.2V (锂电池电压)
  - VCHG: 4.2V (充电时)
```

**阅读原理图的三个步骤:**
1. **找电源**：从输入（USB 5V）出发，追踪电源经过哪些芯片变成了什么电压
2. **找主控**：找到ESP32-S3模组，看它的电源引脚连到哪里，GPIO连到了哪些外设
3. **按模块追踪**：逐个外设阅读，看它用了哪些GPIO，是什么通信接口

### 任务3.2: 焊接DC-DC电源电路 (30分钟)

**焊接顺序非常重要！先电源，后主控，最后外设。这样每焊一部分就能测试，及早发现问题。**

Soldering order matters! Power first, then the main controller, then peripherals. This way you can test each section and catch problems early.

**步骤:**

1. **准备工具和物料**
   - 电烙铁（无铅调350°C / 有铅调310°C）
   - 焊锡丝 (0.6mm)
   - 镊子
   - 助焊剂
   - 主板PCB
   - 物料清单(BOM)中电源部分的元器件

2. **焊接SY8089 DC-DC降压芯片 (SOT-23-5封装)**
   ```
   SY8089引脚识别:
          ┌─────┐
    EN  ──┤1   5├── VIN(5V)
   GND  ──┤2   4├── SW(电感连接点)
    FB  ──┤3   │
          └─────┘
          (底视图)
   
   焊接步骤:
   1. 在PCB上找到U1(SY8089)的位置，注意方向(芯片上的点和PCB上的标记对齐)
   2. 在1个焊盘上预加锡
   3. 用镊子放好芯片，焊接预加锡的焊盘定位
   4. 检查芯片方向和引脚对齐
   5. 逐个焊接其余4个引脚
   ```

3. **焊接DC-DC外围元件**
   - 输入电容(C1): 10uF，注意极性！有极性电容正极接VIN
   - 输出电容(C2, C3): 10uF + 0.1uF
   - 电感(L1): 2.2uH，无极性
   - 反馈电阻(R1, R2): 设置输出电压

4. **焊接TP4054充电管理 (SOT-23-5)**
   ```
   TP4054引脚:
          ┌─────┐
   CHRG ──┤1   5├── VCC (5V)
   GND  ──┤2   4├── BAT (接电池)
   PROG ──┤3   │  (充电电流设置)
          └─────┘
   
   注意: 
   - R3(PROG电阻)决定充电电流: I_charge = 1000V/R_prog
   - 1.6kΩ = 约625mA, 2kΩ = 约500mA
   - 检查BAT端是否正确连接到电池接口
   ```

5. **焊接电源指示灯**
   - D2: 充电指示灯（红色LED），限流电阻R4(1kΩ)
   - D3: 电源指示灯（绿色LED），限流电阻R5(1kΩ)
   - 注意LED极性：长脚为正，短脚为负，PCB上通常有标记

6. **电源部分检查**
   ```
   焊接后检查清单:
   [ ] SY8089方向正确
   [ ] TP4054方向正确
   [ ] 电解电容极性正确(正极接高电压)
   [ ] LED极性正确
   [ ] 所有焊点无连锡
   [ ] 电感焊接牢固(大焊盘需要足够热量)
   
   用万用表蜂鸣档检查:
   [ ] VCC_5V和GND之间不通(不短路!)
   [ ] VCC_3V3和GND之间不通(不短路!)
   ```

**常见问题:**
- **SOT-23-5太小不好焊**：先固定一个引脚，检查对齐，再焊其余引脚。使用助焊剂帮助流动
- **电感焊不上**：电感焊盘大，散热快。提高烙铁温度到370°C，烙铁头接触时间延长到3-4秒
- **LED不亮**：极性焊反了。拆下来换个方向重焊

### 任务3.3: 焊接ESP32-S3模组 (30分钟)

**这是最关键的一步！ESP32-S3模组有40+个引脚，间距1.27mm，需要耐心和技巧。**

This is the most critical step! The ESP32-S3 module has 40+ pins at 1.27mm pitch -- patience and skill required.

**步骤:**

1. **识别模组方向**
   ```
   ESP32-S3模组底部视图 (WROOM-1):
   ┌─────────────────────────────┐
   │ GND                        │  ─┐
   │ 3V3                        │   ├── 左侧引脚
   │ EN                         │   │
   │ IO4 ~ IO7                  │   │
   │ ...                        │  ─┘
   │                   IO8~IO13 │  ─┐
   │                   IO14~IO21│   ├── 右侧引脚
   │                   ...      │   │
   │                   GND      │  ─┘
   └─────────────────────────────┘
   
   PCB丝印上的天线区域标记指示模组方向
   天线部分应朝向PCB边缘(远离其他电路)
   ```

2. **焊接模组**
   ```
   方法一: 逐个引脚焊接 (推荐给新手)
   1. 先在对角位置的两个焊盘上加少量锡
   2. 放好模组，对齐所有引脚
   3. 焊接两个对角定位引脚
   4. 检查对齐情况，必要时调整
   5. 逐个焊接其余所有引脚
   6. 注意相邻引脚不要连锡
   
   方法二: 拖焊 (需要技巧)
   1. 定位两个对角引脚(同上)
   2. 在所有引脚上均匀涂助焊剂
   3. 烙铁头带较多锡，沿引脚根部匀速拖动
   4. 助焊剂会使连锡自动分离
   5. 如有连锡: 加助焊剂，用干净烙铁头重新拖一遍
   ```

3. **焊接后检查**
   ```
   [ ] 模组方向正确(天线区域朝向PCB边缘)
   [ ] 所有引脚无虚焊(用镊子轻轻拨动测试)
   [ ] 无引脚间连锡(用放大镜检查每对相邻引脚)
   [ ] 模组贴合PCB, 无明显翘起
   
   万用表检查:
   [ ] 3.3V和GND之间无短路(阻抗应>100Ω)
   [ ] EN引脚和3.3V之间的上拉电阻存在
   ```

---

## 下午: 上电测试与传感器焊接 | Afternoon: Power-up Test & Sensor Soldering

### 任务3.4: 首次上电测试 (40分钟)

**这是最激动人心的时刻！但在通电之前，必须通过各种检查。**

**步骤:**

1. **上电前检查**
   ```powershell
   用万用表蜂鸣档检查(断电状态):
   
   1. VCC_5V <-> GND: 应不通 (阻抗>1kΩ)
   2. VCC_3V3 <-> GND: 应不通 (阻抗>500Ω)
   3. VBAT <-> GND: 应不通 (阻抗>1kΩ)
   
   如果任何一对是通的(蜂鸣器响)，说明有短路！
   立即停止，检查所有焊接点，找到短路位置。
   ```

2. **连接USB线，测量各点电压**
   ```powershell
   档位: DC 20V
   
   测量点1: VBUS (USB 5V输入)
   预期值: 4.75V ~ 5.25V ✓
   
   测量点2: VCC_3V3 (DC-DC输出)
   预期值: 3.23V ~ 3.37V ✓
   
   测量点3: TP4054 BAT引脚
   预期值: 0V (未接电池时) / 3.7-4.2V (接电池时) ✓
   
   测量点4: ESP32-S3模组3V3引脚
   预期值: 3.23V ~ 3.37V ✓
   
   如果某点电压不对:
   - 无电压: 检查电源通路，可能是虚焊
   - 电压偏低: 可能有短路或高阻抗焊点
   - 电压偏高: DC-DC反馈电阻可能焊错值
   ```

3. **检查LED指示灯**
   - 电源指示灯(绿色)应亮起
   - 充电指示灯(红色): 不接电池时可能微亮或全亮(正常)

4. **连接串口，检查芯片是否响应**
   ```powershell
   # 在VS Code终端或PowerShell中
   idf.py -p COM3 monitor
   
   # 按一下RST按钮(或EN按钮)
   # 应该看到类似输出:
   # ESP-ROM:esp32s3-20210327
   # Build:Mar 27 2021
   # ...
   # 之后可能不断重启(因为没有烧录有效固件)
   ```
   如果看到这个输出，说明ESP32-S3芯片焊接成功、供电正常！

5. **烧录blink测试程序**
   ```c
   // main/blink_test.c
   #include "freertos/FreeRTOS.h"
   #include "freertos/task.h"
   #include "driver/gpio.h"
   #include "esp_log.h"
   
   static const char *TAG = "blink";
   
   // 主板上的LED引脚（根据实际原理图确认）
   #define LED_GPIO GPIO_NUM_48  // ESP32-S3内置LED或外接LED的GPIO
   
   void app_main(void)
   {
       ESP_LOGI(TAG, "SparkBot 主板测试开始!");
       
       // 配置GPIO为输出
       gpio_reset_pin(LED_GPIO);
       gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
       
       while (1) {
           ESP_LOGI(TAG, "LED ON");
           gpio_set_level(LED_GPIO, 1);
           vTaskDelay(1000 / portTICK_PERIOD_MS);
           
           ESP_LOGI(TAG, "LED OFF");
           gpio_set_level(LED_GPIO, 0);
           vTaskDelay(1000 / portTICK_PERIOD_MS);
       }
   }
   ```
   
   ```powershell
   # 编译烧录
   idf.py set-target esp32s3
   idf.py build
   idf.py -p COM3 flash monitor
   ```

**预期结果:**
- 各测试点电压正常
- ESP32-S3识别到，有启动输出
- blink程序成功烧录，串口输出LED状态
- 如果外接LED，可以看到LED闪烁

**常见问题:**
- **串口无输出**：检查USB线是否支持数据传输，尝试不同USB口
- **芯片不断重启**：检查EN引脚是否通过上拉电阻连接到3.3V（通常10kΩ）
- **电压为0**：DC-DC可能焊接有问题，重新检查SY8089及外围元件
- **电源灯不亮但电压正常**：LED焊反了
- **烧录失败**：可能需要手动进入下载模式（按住BOOT -> 按RST -> 松开RST -> 松开BOOT）

### 任务3.5: 焊接BMI270传感器 (20分钟)

**步骤:**

1. **认识BMI270**
   ```
   BMI270: 6轴惯性测量单元(IMU)
   ┌──────────┐
   │   BMI270  │  LGA-14封装 (14个焊盘在底部)
   │           │  尺寸: 2.5×3.0×0.8mm
   │   ┌─┐     │  测量: 3轴加速度 + 3轴陀螺仪
   │   └─┘     │  接口: I2C (地址0x68)
   └──────────┘
   
   这是一个底部焊盘(LGA)的元件，焊接需要热风枪或技巧
   ```

2. **焊接方法(LGA/无引脚封装)**
   ```
   方法一: 热风枪焊接 (推荐)
   1. 在焊盘上涂薄薄一层锡浆/助焊剂
   2. 用镊子精确放置BMI270，对准PCB丝印方向
   3. 热风枪设置280°C，中低风速
   4. 均匀加热芯片区域，约20-30秒
   5. 看到芯片轻微"复位"动作(锡熔化后表面张力自动对齐)
   6. 冷却1分钟后检查
   
   方法二: 烙铁焊接
   1. 在PCB焊盘上薄薄上一层锡
   2. 涂助焊剂
   3. 放置芯片对准
   4. 烙铁接触PCB焊盘边缘(不直接接触芯片)
   5. 热量传到焊盘，锡熔化固定芯片
   6. 逐个焊盘处理
   ```

3. **焊接后检查**
   ```
   [ ] 芯片方向正确(芯片上的标记和PCB丝印一致)
   [ ] 无连锡(LGA封装不易连锡，但仍需检查)
   [ ] 芯片贴合PCB，无明显歪斜
   
   万用表检查:
   [ ] VDDIO和GND之间无短路(阻抗>10kΩ)
   [ ] SDA/SCL上拉电压正常(3.3V)
   ```

**常见问题:**
- **热风枪把旁边的小元件吹跑了**：风速调小，或先用高温胶带保护周围元件
- **芯片没对正**：重新加热，用镊子轻轻推动校正
- **虚焊(LGA特别容易虚焊)**：BMI270底部焊盘是否都有接触，必要时重新热风焊接

### 任务3.6: 原理图追踪练习 (20分钟)

拿出SparkBot原理图，完成以下追踪练习：

1. **找到GPIO48**：从原理图中找到GPIO48连到了哪个元器件(LED)
2. **追踪I2C总线**：找到BMI270的SDA和SCL分别连到了ESP32的哪个GPIO
3. **追踪电源**：从USB VBUS出发，经过哪些元件到了ESP32的3V3供电
4. **找到BOOT和EN**：找到BOOT按钮和EN按钮分别连到哪些GPIO

---

## 今日作业 | Homework

### 必做题
1. 确保主板电源正常：测量并记录各测试点的电压值
2. 确保ESP32-S3模组焊接正确：成功烧录blink程序并看到LED闪烁
3. 完成BMI270传感器焊接
4. 在原理图上标注出5个关键测试点的位置

### 挑战题
1. 用手机微距拍一张ESP32-S3模组的焊接照片，检查每个引脚的对齐和焊点质量
2. 阅读SY8089数据手册，计算R1/R2的值如何得到3.3V输出 (公式: Vout = 0.6 × (1 + R1/R2))
3. 研究BMI270的I2C地址：为什么是0x68？如果SDO引脚接高电平会变成什么地址？

### 思考题
1. DC-DC降压和线性稳压(LDO)有什么区别？为什么SparkBot用DC-DC而不用LDO？
2. 为什么焊接顺序是"电源 -> 主控 -> 外设"？如果反过来先焊传感器再焊电源会有什么问题？
3. ESP32-S3正常工作时EN引脚应该是什么电平？如果没有上拉电阻会怎样？

---

## 明日预告 | Tomorrow's Preview

明天我们将完成SparkBot的剩余硬件组装——焊接显示屏、摄像头、音频电路、触摸FPC和3D打印外壳安装。明天结束时，你将拥有一台外壳完整、所有硬件就位的SparkBot！

Tomorrow we complete the remaining hardware assembly -- soldering the display, camera, audio circuit, touch FPC, and mounting the 3D-printed case. By end of day, you will have a fully assembled SparkBot!

---

## 参考资源 | References

- [原理图阅读入门（B站搜索）](https://search.bilibili.com/all?keyword=%E5%8E%9F%E7%90%86%E5%9B%BE%E9%98%85%E8%AF%BB%E5%85%A5%E9%97%A8)
- [SY8089 数据手册](https://datasheet.lcsc.com/lcsc/2108132002_Shenzhen-Silergy-Corp-SY8089A1AAC_C2775958.pdf)
- [TP4054 数据手册](https://datasheet.lcsc.com/lcsc/1810101113_TPOWER-TP4054_C325651.pdf)
- [BMI270 数据手册](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bmi270-ds000.pdf)
- [ESP32-S3-WROOM-1 模组规格书](https://www.espressif.com/sites/default/files/documentation/esp32-s3-wroom-1_wroom-1u_datasheet_cn.pdf)
- [【保姆级教程】手把手教你焊接ESP32开发板](https://www.bilibili.com/video/BV1jM4y1S7Nf/)

*最后更新：2026-05-27*
