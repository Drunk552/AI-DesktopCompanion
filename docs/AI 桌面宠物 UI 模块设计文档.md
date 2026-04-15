# 🧠 AI 桌面宠物 UI 模块设计文档（树莓派嵌入式版）

---

# 📌 一、设计目标

本 UI 系统用于 AI 桌面宠物项目，目标不是传统工具界面，而是：

> ✅ 构建“有情绪、有记忆、有关系”的 AI 伴侣交互界面

---

## 🎯 设计原则

- 模块化（便于拆分开发）
- 低性能消耗（适配树莓派）
- 情绪表达优先（而非复杂功能）
- 可扩展（后期增加功能无需重构）

## AI桌宠整体系统架构图（Brain → UI 全链路）

┌────────────────────────────────────────────┐
│                用户输入层                  │
│  点击 / 输入 / 拖动 / 摄像头表情           │
└────────────────────────────────────────────┘
                    │
                    ▼
┌────────────────────────────────────────────┐
│          UI 输入层（ui_input_handler）      │
│  - 按钮事件                                │
│  - 输入框内容                              │
│  - 菜单选择                                │
└────────────────────────────────────────────┘
                    │ emit
                    ▼
┌────────────────────────────────────────────┐
│                Event Bus                   │
│        （系统唯一通信通道）                │
└────────────────────────────────────────────┘
                    │
                    ▼
┌────────────────────────────────────────────┐
│           🧠 Brain（核心控制器）           │
│  controller/brain.cpp                      │
│                                            │
│  - 情绪分析（emotion）                     │
│  - 人格策略（personality）                 │
│  - Prompt构造（prompt）                    │
│  - AI调用（gemma）                         │
│  - 记忆读写（sqlite）                      │
└────────────────────────────────────────────┘
                    │ emit
                    ▼
┌────────────────────────────────────────────┐
│                Event Bus                   │
└────────────────────────────────────────────┘
                    │
        ┌───────────┼────────────┬────────────┐
        ▼           ▼            ▼            ▼
┌────────────┐ ┌────────────┐ ┌────────────┐ ┌────────────┐
│ ui_pet     │ │ ui_chat    │ │ ui_status  │ │ notification│
│（表情）     │ │（聊天）     │ │（状态）     │ │（气泡）     │
└────────────┘ └────────────┘ └────────────┘ └────────────┘
        │           │            │            │
        └───────────┴────────────┴────────────┘
                    │
                    ▼
┌────────────────────────────────────────────┐
│             LVGL 渲染层（UI）              │
└────────────────────────────────────────────┘

✅ 1. 只有一条数据通道：EventBus
所有东西都必须走：
UI → EventBus → Brain → EventBus → UI

✅ 2. Brain 是唯一“思考者”
UI 不思考
UI_controller 不思考
只有 Brain 思考

✅ 3. UI 是“被动更新”
// UI只监听
subscribe("emotion.update")
subscribe("new.message")

## 完整事件流示例（真实运行逻辑）

🎬 场景：用户说“我今天很累”
① UI层
g_event_bus.emit("user.input.text", "我今天很累");

② Brain 接收
subscribe("user.input.text")
→ 做：

情绪分析
记忆记录
构造 prompt
调用 AI

③ Brain 输出
g_event_bus.emit("ai.response.text", "……你以前不会这样说");
g_event_bus.emit("emotion.update", "sad");
g_event_bus.emit("affection.update", "2");

④ UI 自动更新
chat_panel → 显示文字
pet_layer → 切换表情
status_bar → 更新好感度

## 事件命名规范（非常重要）

❗核心规则（必须遵守）

✅ 规则1：统一格式
模块.行为.对象

✅ 规则2：全部小写 + 点分隔
user.input.text   ✅
UserInput         ❌
INPUT_TEXT        ❌

✅ 规则3：语义明确（不能模糊）
emotion.update    ✅
update            ❌
data_change       ❌

## 完整事件分类（直接可用）

🧍 用户输入类
user.input.text          # 用户输入文本
user.input.voice         # 语音输入
user.action.click        # 点击行为
user.action.drag         # 拖动桌宠
user.action.open_menu    # 打开菜单
user.action.switch_page  # 页面跳转

🧠 AI / Brain 输出类
ai.response.text         # AI回复
ai.response.stream       # 流式输出
ai.thinking.start        # 开始思考
ai.thinking.end          # 思考结束

🎭 情绪系统
emotion.update           # 当前情绪变化
emotion.detected         # 视觉识别到情绪
emotion.state.change     # 情绪状态变化（平静→难过）

❤️ 关系系统
affection.update         # 好感度变化
affection.level.change   # 等级变化
relationship.update      # 关系状态变化

🧠 记忆系统
memory.add               # 新记忆
memory.key.add           # 重要记忆
memory.update            # 记忆更新
memory.load              # 加载历史

🖥️ UI控制类
ui.screen.switch         # 页面切换
ui.overlay.show          # 显示悬浮层
ui.overlay.hide          # 隐藏
ui.notification.show     # 弹出提示

⚙️ 系统类
system.camera.on
system.camera.off
system.mode.change
system.shutdown

---

# 🧱 二、整体 UI 架构

```text
主界面（Main UI）
   ↓ 按钮跳转
主菜单（Menu UI）
   ↓
各功能子页面（Modules）

📂 推荐 UI 目录结构

ui/
├── common/                      # [通用组件层] 样式、公共控件
│   ├── ui_style.cpp             # 颜色、字体
│   ├── ui_widgets.cpp           # 封装好的控件
│   └── ui_anim.cpp              # （动画封装）

├── managers/                    # [核心管理层] 负责页面跳转
│   ├── ui_manager.cpp           # 页面切换（核心）
│   └── ui_router.cpp            # （页面路由）

├── controller/              # ui业务桥接
│   ├── ui_controller.cpp    # 负责：UI初始化/UI整体调度
│   └── ui_input_handler.cpp # 专门处理输入

├── screens/                     # [页面层]
│
│   ├── home/                    # 主界面
│   │   ├── ui_scr_home.cpp
│   │   └── ui_scr_home.h
│
│   ├── menu/                    # 主菜单
│   │   ├── ui_scr_menu.cpp
│   │   └── ui_scr_menu.h
│
│   ├── persona/                 # ❤️ 1.人格系统模块
│   │   ├── ui_persona.cpp
│   │   └── ui_persona.h
│
│   ├── memory/                  # 🧠 2.记忆系统模块
│   │   ├── ui_memory.cpp
│   │   └── ui_memory.h
│
│   ├── tools/                   # 🧰 3.工具模块
│   │   ├── ui_tools.cpp
│   │   └── ui_tools.h
│
│   ├── settings/                # ⚙️ 4.系统设置模块
│   │   ├── ui_settings.cpp
│   │   └── ui_settings.h
│
│   ├── interaction/             # 🎮 5.扩展互动模块
│   │   ├── ui_interaction.cpp
│   │   └── ui_interaction.h

├── overlay/                 # （关键）
│   ├── ui_pet_layer.cpp     # 桌宠悬浮层
│   ├── ui_chat_panel.cpp    # 聊天层
│   ├── ui_status_bar.cpp    # 状态栏
│   └── ui_notification.cpp  # 气泡提示

├── ui_app.cpp                   # UI入口

🖥️ 三、主界面设计（Main UI）

📐 布局结构
┌────────────────────────────┐
│        状态栏（Status）     │
├──────────────┬─────────────┤
│              │             │
│   桌宠层     │   聊天区     │
│   Pet UI     │   Chat UI   │
│              │             │
├──────────────┴─────────────┤
│   输入框 + 菜单按钮         │
└────────────────────────────┘

📌 功能组成
1️⃣ 桌宠层（ui_pet）
显示角色
表情变化（开心/难过/生气）
Idle 动画（呼吸/眨眼）

2️⃣ 聊天模块（ui_chat）
对话气泡
输入框
AI流式输出（逐字）

3️⃣ 状态栏（ui_status）
当前情绪
好感度等级
状态描述（冷淡 / 依赖）

4️⃣ 菜单入口
一个按钮 → 跳转主菜单

📱 四、主菜单设计（Menu UI）

🧩 模块分类（核心结构）
主菜单
├── ❤️ 关系与人格
├── 🧠 记忆与成长
├── 🧰 实用工具
├── ⚙️ 系统设置
└── 🎮 扩展互动

❤️ 五、关系与人格模块

文件：ui_persona.cpp

📌 功能
1️⃣ 人格切换
不同角色（前任 / 默认 / 自定义）

2️⃣ 宠物信息
角色：前任
性格：克制 / 疏离 / 吃醋
当前状态：有点动摇

3️⃣ 关系状态（重要）
好感度：Lv2
关系：若即若离
她对你：还在观察

4️⃣ 人格变化趋势（建议）
最近变化：
→ 更温和
→ 更愿意回应

🧠 六、记忆与成长模块

文件：ui_menu_memory.cpp
📌 子模块

1️⃣ 记忆回顾（基础）
聊天历史记录

2️⃣ 重要记忆（推荐）
她记住的：

- 你说你很累
- 你喜欢下雨
- 你昨天没理她
👉 实现建议：
ALTER TABLE chat ADD COLUMN is_key_memory INTEGER;

3️⃣ 关系时间线（高级）
Day1：初识（冷淡）
Day3：开始回应
Day7：出现情绪波动

🧰 七、实用工具模块

文件：ui_menu_tools.cpp

📌 功能

1️⃣ 翻译
输入文本 → 翻译结果

2️⃣ 日期 / 天气
今天：周三
天气：小雨

AI：……你不是很喜欢这种天气吗？
👉 关键点：AI参与结果表达

3️⃣ 备忘录（强烈推荐）
TODO：

- 写代码
- 吃饭
👉 AI可提醒：

“你又忘了。”

⚙️ 八、系统设置模块
文件：ui_menu_settings.cpp

📌 功能

系统控制
摄像头开关
表情识别开关
模型选择（Gemma等）

性能设置（嵌入式重点）
动画开关
帧率限制
低功耗模式

UI设置
主题（深色 / 冷色）
字体大小

🎮 九、扩展互动模块
文件：ui_menu_interaction.cpp

📌 功能

1️⃣ 主动对话模式
“……你今天很安静。”

2️⃣ 打扰模式
模式：
[ 安静 ]
[ 正常 ]
[ 粘人 ]

3️⃣ 轻互动
点击 → 反应
拖动 → 情绪变化

💬 十、通知系统（Notification）
文件：ui_notification.cpp

📌 功能
主动气泡
情绪提示
“……你今天没怎么说话。”

🔄 十一、模块通信设计

📌 推荐架构
brain → event_bus → ui modules

📌 示例
struct UIEvent {
    std::string type;
    std::string data;
};

📌 事件类型建议
emotion.update
new.message
affection.change
memory.update

## 🎭 UI驱动模式（重要）
本系统采用“状态驱动UI”，而非传统点击驱动：

传统UI：
点击 → 页面变化

本系统：
情绪变化 → UI变化
AI回复 → UI变化

## 🧱 Overlay 生命周期
- 初始化：ui_app_init
- 常驻：不随 screen 切换销毁
- 更新：通过 event_bus

## 🔄 事件方向规范
UI → Brain：只允许 user.* 事件
Brain → UI：只允许 ai.* / emotion.* / affection.*

禁止：
UI直接调用AI
Brain直接操作UI



🚀 十二、开发优先级

✅ 第一阶段（基础可运行）
主界面（ui_main）
聊天模块（ui_chat）
桌宠（ui_pet）
主菜单（ui_menu）

✅ 第二阶段（体验提升）
状态栏（ui_status）
通知系统（ui_notification）
人格页面

🔥 第三阶段（核心差异）
重要记忆
关系时间线
主动对话模式

⚠️ 十三、嵌入式优化建议（重点）

性能优化
动画尽量简单（alpha + position）
避免大图频繁刷新
使用缓存（LVGL buffer）

UI策略
少层级（避免复杂嵌套）
少实时重绘
控制 FPS（保持60帧流程 ）

🎯 十四、总结

❌ 不要做
复杂工具集合 UI
信息密度过高界面

✅ 要做
情绪表达
关系变化
记忆可视化

🧠 最终目标

构建一个：
“不是工具，而是会陪伴、会变化、会记住你的存在”


