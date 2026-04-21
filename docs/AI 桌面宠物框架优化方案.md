# AI 桌面宠物框架优化方案

## 1. 文档目的

本文用于分析当前 `ai_pet` 工程的整体框架结构，并给出后续可执行的优化方向。

重点回答两个问题：

1. 当前工程框架有哪些优点、问题和优化空间
2. 是否适合采用“感知层 / UI层 / AI层 / 动作层”这样的架构

本文件只做分析与方案设计，不直接涉及代码改动。

---

## 2. 当前工程结构分析

### 2.1 当前主要目录分层

当前 `ai_pet` 基本已经形成如下结构：

```text
main.cpp

controller/
├── application.*        # 程序总编排
├── module_registry.*    # 模块装配
└── session.*            # 对话会话与主要业务流程

config/
logger/

vision/
├── camera.*
├── face.*
├── emotion.*
└── vision_pipeline.*

ai/
├── gemma.*
├── prompt.*
├── affinity.*
├── persona_loader.*
└── fallback_personality.*

memory/
└── sqlite.*

ui/
├── ui_app.*
├── ui.*
├── event_bus.*
├── common/
├── controller/
├── managers/
├── overlay/
└── screens/
```

### 2.2 当前启动链路

当前程序启动路径大致如下：

```text
main
 -> Application
 -> ModuleRegistry 装配基础模块
 -> Session 建立会话业务
 -> UI / Vision / AI / Memory 协同运行
```

UI 模式下的主要流程是：

```text
Application::runUIMode
 -> 初始化 UI
 -> 初始化视觉
 -> 初始化 Session
 -> 订阅 UI EventBus
 -> 启动摄像头线程 / AI异步任务
 -> 进入 UI tick 循环
```

### 2.3 当前结构的优点

#### 1. 已经具备明确的模块边界

视觉、AI、记忆、UI、配置、日志已经按职责拆目录，整体不是混乱工程。

#### 2. `ModuleRegistry` 集中装配模块

这使得依赖创建逻辑没有散落在 `main` 或 UI 代码里，降低了初始化复杂度。

#### 3. `Session` 已经成为业务主入口

用户输入到 AI 回复的主要闭环已经在 `Session` 中收敛，说明工程已经开始形成“业务中枢”。

#### 4. UI 层正在变得模块化

当前 UI 已经拆成：

- `ui_app`
- `ui_controller`
- `ui_manager`
- `ui_screen_lifecycle`
- `screens/*`
- `overlay/*`

这个方向是正确的。

#### 5. 已经开始关注运行稳定性

例如屏幕生命周期管理器通过延迟异步销毁页面，避免事件处理中删除对象导致崩溃，这说明框架开始从“能跑”走向“可维护”。

---

## 3. 当前框架的主要问题

### 3.1 `Application` 职责过重

当前 `Application` 同时承担：

- 模式入口选择
- UI 模式运行循环
- 摄像头线程管理
- AI 异步任务管理
- UI 事件订阅
- 生命周期与退出控制

这会导致：

- 后续加模式时继续膨胀
- 线程调度和业务调度耦合
- 难以做单元测试

### 3.2 `ModuleRegistry` 是方便但偏松的依赖访问方式

`ModuleRegistry` 当前本质上更像一个 Service Locator。

优点是简单直接，缺点是：

- 上层可以轻易拿到所有模块
- 容易形成跨层直接访问
- 长期不利于边界治理和替换实现

### 3.3 `Session` 已开始承担过多职责

当前 `Session` 同时处理：

- prompt 构造
- AI 调用
- 记忆读写
- 好感更新
- 当前情绪维护
- persona 使用

这意味着 `Session` 既像应用服务，又像领域服务，还承担了一部分状态管理职责。

### 3.4 事件总线归属不够清晰

当前 EventBus 位于 `ui/` 中，但 `Application` 与业务逻辑也在依赖它。

这会造成语义反转：

- 看起来像 UI 拥有总线
- 但实际上总线已经承担了系统协调职责

如果继续演进，建议把“系统级事件总线”从 UI 中独立出来。

### 3.5 UI 层现在已有结构，但命名还不够收敛

当前存在：

- 对外的 `UIManager`
- UI 内部的 `pet_ui::manager::UiManager`
- `UiController`

语义上存在一点重叠，后续维护者需要花时间区分它们的职责。

### 3.6 页面切换仍有字符串驱动问题

像 `switchScreen("home")` 这种方式虽然灵活，但长期容易带来：

- 拼写错误
- 重构困难
- 缺少编译期校验

---

## 4. 推荐的优化方向

### 4.1 第一优先级：瘦身 `Application`

建议将当前 `Application` 的运行逻辑拆成更清晰的运行器：

```text
Application
├── UIModeRunner
├── ChatModeRunner
├── CameraModeRunner
└── FullModeRunner
```

这样 `Application` 只负责“选择模式”，而不负责具体模式内部线程与事件调度。

### 4.2 第二优先级：把系统事件总线从 UI 层上移

建议后续引入一个更高层的事件总线，例如：

```text
runtime/
└── app_event_bus.*
```

让：

- UI 发布 `user.*`
- Brain/AI 发布 `ai.*`
- 感知模块发布 `emotion.*` / `vision.*`

这样事件总线属于系统运行时，而不是 UI 私有模块。

### 4.3 第三优先级：继续拆分 `Session`

建议把 `Session` 的职责逐步拆成：

- 会话编排服务
- 情绪状态服务
- 好感关系服务
- Prompt 生成服务
- 记忆上下文服务

不一定要一步拆成很多类，但逻辑上应开始分层。

### 4.4 第四优先级：页面切换统一成枚举或路由对象

当前 UI 已经有 `ScreenType`，后续建议统一：

- 对外使用明确的 screen id 枚举
- 对内再映射为具体页面加载函数

这样比字符串更稳。

### 4.5 第五优先级：明确“编排层”和“领域层”的边界

建议以后坚持一个原则：

- 编排层负责：线程、调度、模式、事件路由
- 领域层负责：情绪、关系、记忆、回复策略
- UI 层负责：显示与输入

不要让 UI 再逐渐回流成为“半业务层”。

---

## 5. 你提出的“四层架构”是否合适

你提到的结构是：

```text
感知层
UI层
AI层
动作层
```

这个方向是可行的，而且很适合“桌宠”这类交互型系统。

但如果直接这样切，容易有一个问题：

**它更像“功能域切分”，还缺一个上层的编排层。**

换句话说，这四层本身是好的，但还需要一个“脑 / 调度 / 应用编排层”把它们串起来。

### 5.1 我对这四层的理解

#### 1. 感知层

负责从外部世界采集状态。

在你这个项目里，对应：

- `vision/camera.*`
- `vision/face.*`
- `vision/emotion.*`
- `vision/vision_pipeline.*`

未来也可以纳入：

- 麦克风输入
- 语音识别
- 系统时间/天气/系统状态

#### 2. UI层

负责输入输出界面。

对应：

- `ui/` 全部

它应当是被动层：

- 收集用户输入
- 展示宠物状态
- 展示聊天和通知

不应承担思考。

#### 3. AI层

负责智能决策与语义生成。

在你这个项目里，对应：

- `ai/gemma.*`
- `ai/prompt.*`
- `ai/affinity.*`
- `ai/persona_loader.*`
- 一部分 `Session`

但严格来说，目前 AI 层和业务层还混在一起。

#### 4. 动作层

这是很适合桌宠项目的一个概念。

动作层不是“底层驱动”，而是：

- 表情切换
- 动画切换
- 通知弹出
- 主动说话
- 互动行为反馈

如果未来你要做得更像桌宠，这层会非常关键。

它本质上是“把 Brain 决策翻译成外显行为”的层。

---

## 6. 我对这套架构的建议

### 6.1 不建议只保留四层

如果只有：

```text
感知层 / UI层 / AI层 / 动作层
```

那么你很快会遇到一个问题：

**谁来统一调度？**

例如：

- 摄像头识别到用户疲惫
- Memory 里查到最近连续低情绪
- AI 决定先不主动打扰
- 动作层播放“安静陪伴”动画
- UI 状态栏更新为“观察中”

这整条链如果没有一个上层编排者，最后就会重新散落到各模块互相直接调用。

### 6.2 更适合你的，是“五层/五域”结构

我更推荐你将它整理成：

```text
1. 感知层 Perception
2. UI层 Presentation
3. 脑/编排层 Brain / Orchestrator
4. AI与记忆层 Intelligence
5. 动作层 Action / Behavior
```

或者换一种更工程化的表达：

```text
runtime/        # 应用运行时、事件总线、模式切换、线程调度
perception/     # 摄像头、表情识别、语音、环境输入
brain/          # 情绪状态机、关系状态机、事件编排、决策中枢
intelligence/   # LLM、prompt、persona、memory
action/         # 表情、动画、通知、主动互动、行为执行
ui/             # LVGL 页面与显示
```

这个版本更适合桌宠项目长期演进。

---

## 7. 为什么“动作层”值得保留

在传统业务系统里，动作层常常没必要单独提。

但在桌宠项目里，动作层非常有价值，因为它可以把“说什么”与“怎么表现”分开。

例如：

AI层输出：

```text
reply = "……你今天看起来很累"
emotion = "sad"
relationship = "关心"
```

动作层可以进一步决定：

- 宠物切到低落表情
- 对话出现前先停顿 300ms
- 通知气泡不弹出，避免太打扰
- 状态栏关系文案切换成“在意你”

这样工程会更像“桌宠行为系统”，而不是单纯聊天 UI。

---

## 8. 推荐的长期目标结构

如果以后要往更稳定的方向走，我建议你最终形成类似结构：

```text
main.cpp

app/
├── application.*
├── mode_runner_ui.*
├── mode_runner_chat.*
├── module_registry.*
└── app_event_bus.*

perception/
├── camera.*
├── face.*
├── emotion.*
└── vision_pipeline.*

brain/
├── brain_controller.*
├── state_manager.*
├── relation_manager.*
└── event_router.*

intelligence/
├── llm_client.*
├── prompt_builder.*
├── persona_service.*
└── memory_service.*

action/
├── pet_action_controller.*
├── chat_action_controller.*
├── notification_action_controller.*
└── behavior_policy.*

ui/
├── ui_app.*
├── controller/
├── managers/
├── overlay/
└── screens/
```

---

## 9. 结论

### 9.1 当前工程是否需要重做框架

不需要。

当前工程已经具备可继续演进的基础，重点是逐步治理边界，而不是推倒重来。

### 9.2 当前最值得优化的点

优先级建议如下：

1. 拆 `Application`
2. 将系统事件总线上移出 UI
3. 瘦身 `Session`
4. 明确 Brain/编排层
5. 将“动作层”从纯 UI 表现中独立出来

### 9.3 关于“感知层 / UI层 / AI层 / 动作层”

这个架构方向是对的，尤其适合桌宠。

但建议不要直接只保留四层，而是增加一个：

**Brain / Orchestrator 层**

最终更推荐的结构是：

```text
感知层
UI层
Brain编排层
AI与记忆层
动作层
```

这样更符合你文档里“Brain 是唯一思考者”的原则，也更适合后面继续扩展主动互动、行为策略、情绪驱动 UI 和桌宠动作。

---

## 10. 下一步建议

如果后续开始做框架治理，建议按这个顺序推进：

1. 先把 `Application` 拆成多个 mode runner
2. 再把 EventBus 提升为系统级运行时模块
3. 然后引入 `BrainController`，把 `Session` 逐步瘦身
4. 最后再把“动作层”单独落到代码目录里

这个顺序风险最小，也不会打断你当前 UI 和业务功能的继续开发。
