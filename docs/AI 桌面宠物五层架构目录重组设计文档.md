# AI 桌面宠物五层架构目录重组设计文档

## 1. 文档目的

本文用于给 `AI-DesktopCompanion/ai_pet` 提供一份基于“五层架构”的目录重组设计方案。

目标不是立即重写全部代码，而是：

1. 明确长期推荐结构
2. 说明现有目录应该如何迁移
3. 给出低风险、可分阶段推进的重组路径

---

## 2. 推荐的五层架构

建议将工程逐步整理为以下五层：

1. 感知层 `perception`
2. UI层 `ui`
3. Brain 编排层 `brain`
4. AI 与记忆层 `intelligence`
5. 动作层 `action`

同时保留一个应用运行时层：

6. 应用层 `app`

所以更完整的工程表达应当是：

```text
app
perception
brain
intelligence
action
ui
shared
```

其中：

- 五层架构是核心业务结构
- `app` 是运行入口和模式调度层
- `shared` 是跨层基础设施层

---

## 3. 各层职责定义

### 3.1 `app/` 应用层

负责：

- 程序入口后的模式选择
- 模块初始化与装配
- 系统事件总线
- UI 模式 / Chat 模式 / Camera 模式运行器
- 生命周期与线程协调

这一层不负责具体业务决策。

### 3.2 `perception/` 感知层

负责：

- 摄像头输入
- 人脸检测
- 表情识别
- 视觉流水线
- 未来可扩展麦克风、语音识别、系统状态感知

这一层只负责“看见 / 听见 / 获取环境状态”，不负责“怎么回应”。

### 3.3 `brain/` 编排层

负责：

- 统一处理系统事件
- 接收用户输入和感知结果
- 决定何时调用 AI
- 决定何时更新关系/情绪/动作
- 将输入翻译成系统内部决策
- 将决策翻译成 UI 与动作事件

这一层是整个系统唯一的“思考协调者”。

### 3.4 `intelligence/` AI 与记忆层

负责：

- LLM 调用
- Prompt 构建
- Persona 加载
- 长短期记忆
- 好感度与关系计算
- 情绪趋势与上下文提取

这一层提供智能能力，但不直接驱动 UI。

### 3.5 `action/` 动作层

负责：

- 宠物表情切换
- 动画策略
- 主动打扰/安静/陪伴行为
- 通知策略
- 互动反馈行为

动作层不是 LVGL 页面，而是“行为执行器”。

### 3.6 `ui/` UI层

负责：

- 界面渲染
- 输入收集
- 页面切换
- Overlay 常驻层
- 聊天展示与状态展示

UI 层只展示和发输入事件，不做业务推理。

---

## 4. 推荐的目标目录结构

```text
ai_pet/
├── main.cpp
│
├── app/
│   ├── application.h
│   ├── application.cpp
│   ├── app_event_bus.h
│   ├── app_event_bus.cpp
│   ├── module_registry.h
│   ├── module_registry.cpp
│   ├── mode_runner_ui.h
│   ├── mode_runner_ui.cpp
│   ├── mode_runner_chat.h
│   ├── mode_runner_chat.cpp
│   ├── mode_runner_camera.h
│   ├── mode_runner_camera.cpp
│   ├── mode_runner_full.h
│   └── mode_runner_full.cpp
│
├── perception/
│   ├── camera.h
│   ├── camera.cpp
│   ├── face.h
│   ├── face.cpp
│   ├── emotion.h
│   ├── emotion.cpp
│   ├── vision_pipeline.h
│   └── vision_pipeline.cpp
│
├── brain/
│   ├── brain_controller.h
│   ├── brain_controller.cpp
│   ├── conversation_orchestrator.h
│   ├── conversation_orchestrator.cpp
│   ├── emotion_state.h
│   ├── emotion_state.cpp
│   ├── relation_state.h
│   ├── relation_state.cpp
│   ├── event_router.h
│   └── event_router.cpp
│
├── intelligence/
│   ├── llm/
│   │   ├── gemma.h
│   │   └── gemma.cpp
│   ├── prompt/
│   │   ├── prompt.h
│   │   └── prompt.cpp
│   ├── persona/
│   │   ├── persona_loader.h
│   │   └── persona_loader.cpp
│   ├── memory/
│   │   ├── memory_db.h
│   │   └── memory_db.cpp
│   └── relation/
│       ├── affinity.h
│       └── affinity.cpp
│
├── action/
│   ├── pet_action_controller.h
│   ├── pet_action_controller.cpp
│   ├── notification_action_controller.h
│   ├── notification_action_controller.cpp
│   ├── behavior_policy.h
│   └── behavior_policy.cpp
│
├── ui/
│   ├── ui.h
│   ├── ui.cpp
│   ├── ui_app.h
│   ├── ui_app.cpp
│   ├── common/
│   ├── controller/
│   ├── managers/
│   ├── overlay/
│   ├── screens/
│   └── assets/
│
├── shared/
│   ├── config/
│   ├── logger/
│   ├── types/
│   └── utils/
│
├── personas/
├── models/
└── third_party/
```

---

## 5. 现有目录到目标结构的映射

### 5.1 当前 `controller/`

现状：

- `application.*`
- `module_registry.*`
- `session.*`

建议迁移：

- `application.*` -> `app/`
- `module_registry.*` -> `app/`
- `session.*` -> 不建议原样保留

`session.*` 建议拆分后迁移到：

- `brain/conversation_orchestrator.*`
- `brain/emotion_state.*`
- `brain/relation_state.*`

如果短期不拆，可先临时迁移为：

- `brain/session.*`

作为过渡形态。

### 5.2 当前 `vision/`

现状：

- `camera.*`
- `face.*`
- `emotion.*`
- `vision_pipeline.*`

建议直接迁移到：

- `perception/`

这是最清晰、最自然的一层。

### 5.3 当前 `ai/`

现状：

- `gemma.*`
- `prompt.*`
- `affinity.*`
- `persona_loader.*`
- `fallback_personality.*`

建议拆到：

- `intelligence/llm/`
- `intelligence/prompt/`
- `intelligence/relation/`
- `intelligence/persona/`

`fallback_personality.*` 如果仍保留，可以放在：

- `intelligence/persona/`

### 5.4 当前 `memory/`

现状：

- `sqlite.*`

建议迁移到：

- `intelligence/memory/`

并建议重命名为：

- `memory_db.*`

这样语义比 `sqlite.*` 更稳定，避免把“实现技术”当成领域命名。

### 5.5 当前 `ui/`

现状：

- 结构已经比较清晰

建议：

- 继续保留在 `ui/`
- 不建议再拆散到别层

但未来应把：

- 系统级 EventBus 从 `ui/` 中移出

UI 内部保留自己的：

- 页面管理器
- 生命周期管理器
- 页面模块

### 5.6 当前 `config/` 与 `logger/`

建议迁移到：

- `shared/config/`
- `shared/logger/`

因为它们不属于某个业务层，而是跨层基础设施。

---

## 6. 为什么要把 `Session` 拆开

当前 `Session` 是整个工程里最需要“从单体类走向多职责模块”的部分。

它目前做了：

- AI 调用等待
- Prompt 组织
- Memory 读写
- 好感更新
- Persona 使用
- 当前情绪维护

建议重组后的职责如下：

### 6.1 `brain/conversation_orchestrator`

负责：

- 接收用户输入
- 组织一轮完整对话流程
- 决定何时调用智能层

### 6.2 `brain/emotion_state`

负责：

- 当前情绪状态
- 感知层输入后的情绪更新

### 6.3 `brain/relation_state`

负责：

- 当前关系状态
- 好感等级解释

### 6.4 `intelligence/memory/memory_db`

负责：

- 数据存储
- 上下文提取

### 6.5 `intelligence/relation/affinity`

负责：

- 好感变化计算

这样以后系统会更容易维护。

---

## 7. Brain 层在这次重组中的核心地位

这个项目如果要长期做成真正的“AI桌宠系统”，最重要的是要把 `Brain` 独立出来。

建议 Brain 层坚持以下原则：

1. 只有 Brain 负责系统级决策
2. UI 不直接调用 AI
3. 感知层不直接驱动 UI
4. 动作层不自己决定行为目标
5. 所有跨层流转尽量通过统一事件总线或明确的 orchestrator

如果不把 Brain 拎出来，后面即使目录改了，代码仍然会继续耦合。

---

## 8. 动作层应如何落地

动作层不是单纯“动画代码”。

建议它未来承担以下内容：

- 宠物表情切换策略
- 宠物 Idle / Happy / Sad / Angry 动作选择
- 主动提示的打扰等级
- 通知出现条件与优先级
- 行为节奏控制

例如：

```text
Brain 输出：
- ai.response.text = "……你今天很累"
- emotion = sad
- relation = caring

动作层决定：
- 宠物切 sad 表情
- 延迟 300ms 再出现聊天气泡
- 不弹主动通知
```

这会让系统从“会聊天”走向“像桌宠”。

---

## 9. 推荐的迁移顺序

不建议一次性全改目录。

建议按下面顺序分阶段迁移：

### 阶段 1：只改应用层与基础目录名

目标：

- `controller/application.*` -> `app/`
- `controller/module_registry.*` -> `app/`
- `vision/` -> `perception/`
- `config/`、`logger/` -> `shared/`

这一阶段主要改目录，不大量拆逻辑。

### 阶段 2：把 `Session` 搬到 Brain 层过渡

目标：

- `controller/session.*` -> `brain/session.*`

先改语义归属，再改内部职责。

### 阶段 3：拆 `Session`

目标：

- 拆出 emotion / relation / orchestrator

### 阶段 4：把 `ai/` 与 `memory/` 收敛为 `intelligence/`

目标：

- 改成按能力域组织，而不是按历史目录遗留组织

### 阶段 5：引入动作层

目标：

- 将表情、通知、行为策略从 UI 表现中剥离

---

## 10. 对 CMake 和 include 路径的建议

目录重组后，建议同步优化 `CMakeLists.txt`：

### 10.1 不再继续使用全局 `include_directories`

建议后续逐步改成：

- `target_include_directories`

这样依赖边界更清晰。

### 10.2 按层组织源文件列表

可以改成：

```cmake
set(APP_SOURCES ...)
set(PERCEPTION_SOURCES ...)
set(BRAIN_SOURCES ...)
set(INTELLIGENCE_SOURCES ...)
set(ACTION_SOURCES ...)
set(UI_SOURCES ...)
set(SHARED_SOURCES ...)
```

这样 CMake 文件本身也会变成架构的一部分，而不是单纯的大列表。

---

## 11. 风险与注意事项

### 11.1 最大风险不是改目录，而是改边界

目录移动本身风险不算最大。

真正的风险是：

- 代码是否仍然跨层直接访问
- 事件流是否仍然混乱
- UI 是否重新承担业务逻辑

### 11.2 不要一开始就把所有概念类都拆出来

如果太早细拆，会把当前工程拆得过细，反而增加维护负担。

建议遵循：

- 先改目录归属
- 再改层边界
- 最后再细拆类

### 11.3 UI 层先不要大范围再搬

当前 UI 刚刚完成模块化重构，建议先稳定一段时间，不要马上继续大搬迁。

---

## 12. 最终结论

### 12.1 最适合当前项目的长期结构

我建议你的项目长期演进方向是：

```text
app + perception + brain + intelligence + action + ui + shared
```

其中真正的五层核心是：

```text
感知层
UI层
Brain 编排层
AI与记忆层
动作层
```

### 12.2 当前最值得做的目录重组顺序

优先级建议：

1. `controller/application`、`module_registry` 迁到 `app/`
2. `vision/` 迁到 `perception/`
3. `session/` 先迁到 `brain/`
4. `ai/` + `memory/` 重组为 `intelligence/`
5. 最后补 `action/`

### 12.3 当前最关键的设计原则

无论目录怎么重组，都建议坚持：

1. Brain 是唯一系统级决策者
2. UI 只负责输入输出
3. 感知层只负责采集状态
4. AI层只提供智能能力
5. 动作层负责行为呈现，不负责思考

如果这五个原则能守住，目录重组才是真正有价值的，而不是单纯改文件夹名字。
