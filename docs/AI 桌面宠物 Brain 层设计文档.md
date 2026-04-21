# AI 桌面宠物 Brain 层设计文档

## 1. 文档目的

本文用于定义 `AI-DesktopCompanion/ai_pet` 工程中的 Brain 层设计。

Brain 层的目标是：

**成为系统唯一的思考与编排中心。**

它不负责界面渲染，不直接承担底层感知采集，也不直接等同于 LLM 调用器，而是负责把：

- 用户输入
- 感知结果
- 记忆状态
- 人格设定
- 当前关系与情绪

统一组织成系统决策，并驱动 UI 与动作层更新。

---

## 2. Brain 层的核心定位

在整个 AI 桌宠架构中，Brain 层的职责不是“做一切”，而是“负责决定”。

可以用一句话概括：

> 感知层负责看见，UI层负责显示，智能层负责生成，动作层负责表现，Brain 层负责决定下一步要做什么。

---

## 3. Brain 层在系统中的位置

推荐的整体关系如下：

```text
用户输入 / 摄像头输入 / 系统状态
        │
        ▼
感知层 / UI层
        │
        ▼
      Brain 层
        │
        ├── 调用 Intelligence 层获取智能结果
        ├── 更新内部情绪与关系状态
        ├── 输出给 Action 层执行行为
        └── 输出给 UI 层展示结果
```

Brain 层必须处于中间位置，不能被 UI 替代，也不能直接退化成 `Session` 的另一个名字。

---

## 4. Brain 层必须遵守的原则

### 4.1 Brain 是唯一系统级思考者

以下模块都不应该替代 Brain 做决策：

- UI
- UI Controller
- 感知层
- LLM Client
- 动作层

这些层只能做输入、执行、展示或能力输出。

### 4.2 Brain 不负责具体渲染

Brain 不应该直接操作 LVGL 对象。

它只能输出语义化事件，例如：

- `ui.chat.append`
- `ui.status.update`
- `action.pet.expression`
- `action.notification.show`

### 4.3 Brain 不直接等于 AI

LLM 只是 Brain 的一部分能力来源，不等于 Brain 本身。

Brain 应该能决定：

- 是否调用 AI
- 什么时候调用 AI
- 是否需要记忆参与
- 是否只是更新状态而不回复

### 4.4 Brain 必须能处理非对话事件

桌宠系统不能只围绕“用户说一句，AI 回一句”。

Brain 还应处理：

- 用户表情变化
- 空闲时间过长
- 用户长时间不理她
- 当前关系状态变化
- 是否要主动打扰

---

## 5. Brain 层的职责范围

建议 Brain 层承担以下职责：

### 5.1 输入接收与事件编排

负责接收：

- UI 输入事件
- 感知事件
- 系统定时事件

并统一决定处理路径。

### 5.2 会话编排

负责组织“一轮对话”流程：

1. 收到输入
2. 查询当前情绪与关系状态
3. 获取记忆上下文
4. 构造智能调用上下文
5. 调用智能层
6. 更新内部状态
7. 触发 UI 与动作输出

### 5.3 情绪状态管理

负责维护系统当前情绪，例如：

- 平静
- 开心
- 难过
- 疏离
- 吃醋

这里不是只存一个字符串，而是一个可解释的系统状态。

### 5.4 关系状态管理

负责维护：

- 好感度
- 关系等级
- 最近互动趋势
- 主动性/依赖性/距离感

### 5.5 动作决策

负责决定：

- 当前应该切什么表情
- 是否弹通知
- 是否主动发起对话
- 是否进入安静陪伴状态

### 5.6 输出分发

将 Brain 内部决策翻译为：

- UI 可理解的输出事件
- Action 层可执行的行为命令

---

## 6. Brain 层不应该做什么

为了边界清晰，Brain 层不应该直接做这些事：

### 6.1 不直接操作 LVGL 控件

错误示例：

```cpp
lv_label_set_text(...)
```

Brain 不应直接接触 UI 对象。

### 6.2 不自己实现摄像头采集

摄像头采集属于感知层。

### 6.3 不直接封装 HTTP 调用

LLM HTTP 请求应属于 Intelligence 层。

### 6.4 不负责动画资源管理

动作资源与 UI 资源不应该由 Brain 直接持有。

---

## 7. 推荐的 Brain 层内部结构

建议 Brain 层初步拆分为以下几个子模块：

```text
brain/
├── brain_controller.*
├── conversation_orchestrator.*
├── emotion_state.*
├── relation_state.*
├── event_router.*
└── brain_types.h
```

### 7.1 `brain_controller`

定位：

- Brain 层主入口
- 对外唯一暴露的 Brain 门面

职责：

- 初始化 Brain 子模块
- 订阅系统事件
- 驱动整体流程

### 7.2 `conversation_orchestrator`

定位：

- 负责一轮完整对话编排

职责：

- 收到用户输入后组织智能调用流程
- 获取情绪、关系、记忆上下文
- 合并成 prompt 上下文
- 请求 Intelligence 层
- 将结果回写状态与输出事件

### 7.3 `emotion_state`

定位：

- 情绪状态容器与管理器

职责：

- 保存当前情绪
- 处理感知事件带来的情绪变化
- 提供情绪趋势判断

### 7.4 `relation_state`

定位：

- 关系状态容器与管理器

职责：

- 保存当前好感度
- 维护关系等级
- 提供关系语义解释

### 7.5 `event_router`

定位：

- 将系统输入事件分类并路由到对应处理器

职责：

- `user.*` -> 对话/交互处理
- `perception.*` -> 感知处理
- `system.*` -> 定时/状态处理

---

## 8. 推荐的 Brain 数据模型

建议 Brain 层内部维护一个统一状态快照，例如：

```text
BrainState
├── current_emotion
├── current_affection
├── current_relation_level
├── current_persona
├── recent_emotion_trend
├── last_user_interaction_at
├── ai_busy
└── active_mode
```

这个状态对象不一定要一步到位实现成完整类，但设计上应该明确存在。

这样做的价值是：

- UI 和动作层拿到的是 Brain 的输出结果
- Brain 自己对当前系统状态有完整视图

---

## 9. 推荐事件流设计

### 9.1 用户输入事件流

```text
UI 输入
 -> app_event_bus 发布 user.input.text
 -> Brain event_router 接收
 -> conversation_orchestrator 编排
 -> Intelligence 层生成回复
 -> Brain 更新 emotion/relation/state
 -> Brain 发布：
    - ui.chat.append
    - ui.status.update
    - action.pet.expression
    - action.notification.show
```

### 9.2 感知输入事件流

```text
Perception 层识别到 emotion = tired
 -> app_event_bus 发布 perception.emotion.detected
 -> Brain emotion_state 更新内部情绪判断
 -> Brain 决定：
    - 是否立即更新 UI
    - 是否改变宠物表情
    - 是否调整主动打扰策略
```

### 9.3 主动行为事件流

```text
系统空闲计时器触发
 -> app_event_bus 发布 system.idle.timeout
 -> Brain 判断当前关系与情绪
 -> Brain 决定是否主动对话
 -> Action 层执行行为
 -> UI 展示主动消息
```

---

## 10. Brain 层与其他层的接口关系

### 10.1 Brain 与 Perception

Perception -> Brain

只传输感知结果，例如：

- 人脸是否出现
- 检测到的情绪
- 摄像头可用状态

Brain 不反向控制 Perception 的内部算法。

### 10.2 Brain 与 Intelligence

Brain -> Intelligence

Brain 向 Intelligence 请求能力：

- 生成回复
- 获取 Persona
- 查询 Memory
- 计算关系变化

Intelligence 返回能力结果，但不自己决定系统下一步。

### 10.3 Brain 与 Action

Brain -> Action

Brain 只下达行为目标：

- 切换表情
- 弹出通知
- 触发主动行为

Action 负责具体执行策略。

### 10.4 Brain 与 UI

Brain -> UI

Brain 只输出展示事件：

- 新消息
- 状态变化
- 页面提示

UI 只负责显示。

---

## 11. 当前工程与 Brain 层的关系分析

当前工程中，最接近 Brain 的模块其实是：

- `controller/session.*`
- `Application::runUIMode()` 中的一部分编排逻辑

但它们现在还不等于真正的 Brain。

原因是：

### 11.1 `Session` 更像对话业务服务，不是系统 Brain

它现在主要聚焦：

- prompt
- AI
- memory
- affinity

但还没有真正承担：

- 感知事件统一编排
- 动作决策
- 主动行为策略
- 系统事件路由

### 11.2 `Application` 承担了太多本该属于 Brain 的编排职责

例如：

- UI 事件订阅
- AI 线程组织
- 摄像头线程与 UI 更新协调

这些未来都应逐步转移到：

- `brain_controller`
- `conversation_orchestrator`
- `event_router`

---

## 12. Brain 层第一版落地建议

不建议一开始就做很重的 Brain 重构。

建议第一版 Brain 只做以下事情：

### 12.1 第一步：将 `Session` 迁入 `brain/` 作为过渡

先形成：

```text
brain/session.*
```

即使内部还没拆，也先把语义归位。

### 12.2 第二步：新增 `brain_controller`

作用：

- 成为 UI / Perception / Intelligence 的中间入口
- 接管原来 `Application` 中部分事件编排

### 12.3 第三步：逐步拆出 `emotion_state` 和 `relation_state`

先把状态相关逻辑从 `Session` 中剥离。

### 12.4 第四步：最后再引入动作决策接口

等动作层开始成型后，再让 Brain 输出更完整的行为命令。

---

## 13. 推荐的 Brain 输出事件类型

建议 Brain 统一输出几类事件：

### UI 事件

- `ui.chat.append`
- `ui.chat.thinking`
- `ui.status.emotion`
- `ui.status.affection`
- `ui.status.relationship`
- `ui.notification.show`

### Action 事件

- `action.pet.expression`
- `action.pet.idle_mode`
- `action.behavior.proactive_chat`
- `action.notification.policy`

### System 事件

- `brain.turn.started`
- `brain.turn.completed`
- `brain.state.changed`

---

## 14. 推荐的 Brain 输入事件类型

### 用户输入类

- `user.input.text`
- `user.action.click`
- `user.action.menu`

### 感知输入类

- `perception.camera.ready`
- `perception.face.detected`
- `perception.emotion.detected`

### 系统输入类

- `system.idle.timeout`
- `system.app.startup`
- `system.app.shutdown`

---

## 15. Brain 层设计中的关键风险

### 15.1 不要让 Brain 重新变成超大类

如果把所有逻辑都塞进 `brain_controller.cpp`，那只是把 `Application` 的问题挪了位置。

### 15.2 不要让 Brain 直接拿 UI 控件指针

一旦 Brain 持有 LVGL 控件指针，边界会立刻失效。

### 15.3 不要把 Brain 和 LLM 混成一个模块

LLM 是能力提供者，不是 Brain 本身。

### 15.4 不要过早把动作层硬编码进 UI 层

动作应该先作为 Brain 的输出语义，再由 Action 层执行。

---

## 16. Brain 层的最终目标

Brain 层最终应达到这样的效果：

1. 所有系统级决策都经过 Brain
2. UI 不再承担业务思考
3. 感知层只负责输入世界状态
4. AI 层只提供智能能力
5. 动作层只执行行为策略

届时整个系统会更接近真正的“桌宠架构”，而不是“聊天程序 + 一层 UI”。

---

## 17. 结论

Brain 层是你这个项目最关键的一层。

它不是可选优化，而是未来从“功能工程”升级为“系统架构”的关键。

建议你后续推进顺序为：

1. 先完成第一阶段目录迁移
2. 再将 `Session` 过渡到 `brain/`
3. 引入 `brain_controller`
4. 逐步拆出 `emotion_state` / `relation_state`
5. 最后再和 `action/` 层联动

一句话总结：

**Brain 层应成为整个桌宠系统唯一的编排与决策中心。**
