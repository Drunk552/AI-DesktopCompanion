# AI 桌面宠物第二阶段 Brain 落地实施方案

## 1. 文档目标

本文用于定义 `ai_pet` 工程在完成第一阶段目录归位之后，如何进入第二阶段：

**将 Brain 层真正落到代码结构中。**

第二阶段的目标不是一步到位做完整 Brain，而是完成一个可运行、可扩展、低风险的 Brain 第一版落地。

---

## 2. 第二阶段的核心目标

第二阶段主要完成以下 4 件事：

1. 把当前 `Session` 从“controller 时代的业务类”迁入 `brain/` 语义域
2. 新增 `brain_controller` 作为 Brain 层对外入口
3. 将当前 `Application` 中部分系统编排逻辑转移到 Brain
4. 为后续拆分 `emotion_state` / `relation_state` / `conversation_orchestrator` 做好过渡结构

一句话概括：

**第二阶段是让 Brain 在工程里“站起来”，而不是立刻把 Brain 全拆完。**

---

## 3. 第二阶段不做什么

为了控制风险，这一阶段明确不做以下事情：

1. 不重写整个事件总线
2. 不立刻引入完整动作层
3. 不一次性把 `Session` 拆成 4 到 5 个类
4. 不重写 UI 层现有结构
5. 不大改 AI / Memory / Vision 的内部实现

第二阶段只做“Brain 归位 + 编排职责迁移 + 过渡层建立”。

---

## 4. 第二阶段完成后的目标结构

第二阶段建议达到这样的目录状态：

```text
ai_pet/
├── app/
│   ├── application.*
│   ├── module_registry.*
│   └── mode runners ...
│
├── brain/
│   ├── brain_controller.h
│   ├── brain_controller.cpp
│   ├── session.h
│   ├── session.cpp
│   ├── brain_types.h
│   └── brain_events.h
│
├── perception/
├── intelligence/
├── ui/
└── shared/
```

注意：

- 这里的 `brain/session.*` 是过渡形态
- 不意味着最终还会保留 `session` 这个名字
- 只是为了让原有逻辑先归属到 Brain 层，而不立即彻底拆碎

---

## 5. 第二阶段推荐新增的 Brain 文件

### 5.1 `brain/brain_controller.h/.cpp`

作用：

- Brain 层对外唯一门面
- 接管原本在 `Application::runUIMode()` 中的一部分编排逻辑

建议职责：

- 初始化 Brain
- 接收 UI 输入事件
- 接收感知事件
- 决定是否触发对话流程
- 将结果发布给 UI 与动作层

### 5.2 `brain/session.h/.cpp`

作用：

- 暂时承接原 `controller/session.*`
- 作为 Brain 内部的会话编排服务

短期目标：

- 文件先迁位置
- 逻辑尽量不大改

长期目标：

- 逐步拆为：
  - `conversation_orchestrator`
  - `emotion_state`
  - `relation_state`

### 5.3 `brain/brain_types.h`

建议新增，用于统一 Brain 层数据结构，例如：

- `BrainState`
- `BrainInput`
- `BrainOutput`
- `BrainTurnResult`

即使第一版内容不多，也建议先把类型入口建立起来。

### 5.4 `brain/brain_events.h`

建议新增，用于集中管理 Brain 相关事件名。

例如：

- `brain.turn.started`
- `brain.turn.completed`
- `brain.state.changed`
- `brain.emotion.changed`

这一步可以减少事件名散落在各处的问题。

---

## 6. 第二阶段建议迁移的代码职责

### 6.1 当前在 `Application` 中的职责

目前 `Application::runUIMode()` 里包含了这些 Brain 候选逻辑：

1. 订阅 `user.input.text`
2. 判断输入是否为空/退出
3. 判断 AI 是否忙碌
4. 启动一轮 AI 处理
5. 在 AI 完成后分发：
   - 回复
   - 情绪
   - 好感
   - 关系
6. 摄像头线程里感知到情绪后推送 UI 事件

这些里面，真正属于 Brain 的部分是：

- 输入事件处理
- 一轮对话编排
- 感知事件解释
- 状态更新输出

### 6.2 迁移目标

第二阶段建议把以下职责从 `Application` 挪到 `BrainController`：

#### A. 用户输入编排

从：

- `Application::runUIMode()` 的事件订阅 lambda

迁到：

- `BrainController::bindUIEvents()`
- `BrainController::handleUserTextInput()`

#### B. AI turn 流程调度

从：

- `Application` 中的 `std::async` 任务编排

迁到：

- `BrainController::startConversationTurn()`

#### C. 感知到情绪后的处理

从：

- 摄像头线程中直接 `session_.onEmotionDetected()` + UI 事件发射

迁到：

- `BrainController::handlePerceptionEmotion()`

这样 UI 和感知层都不再直接决定最终状态传播方式。

---

## 7. 第二阶段建议保留在 `Application` 的职责

即使引入 Brain，`Application` 仍然要保留以下职责：

1. 模式入口选择
2. 模块初始化
3. UI 主循环驱动
4. 摄像头线程生命周期管理
5. 程序退出控制

也就是说，第二阶段后：

- `Application` 仍负责“程序运行”
- `BrainController` 负责“系统决策与编排”

---

## 8. 第二阶段推荐实施步骤

### 步骤 1：创建 `brain/` 目录

先新增：

- `brain/brain_controller.h`
- `brain/brain_controller.cpp`
- `brain/brain_types.h`
- `brain/brain_events.h`

这一步先不删旧逻辑。

### 步骤 2：迁移 `controller/session.*` 到 `brain/session.*`

做法建议：

1. 物理移动文件
2. 修改 include 路径
3. 修改命名空间或头文件引用
4. 保持逻辑基本不变

目标是先让 `Session` 在语义上属于 Brain 层。

### 步骤 3：引入 `BrainController`

第一版 `BrainController` 可以很轻：

- 持有对 `brain::Session` 的引用
- 持有对 EventBus 的引用
- 提供几个公开接口：
  - `init()`
  - `bindUIEvents()`
  - `handleUserTextInput()`
  - `handlePerceptionEmotion()`

### 步骤 4：将 UI 输入处理从 `Application` 挪入 `BrainController`

迁移：

- `user.input.text` 订阅
- turn busy 判断
- AI 处理中提示事件
- 回复完成后的 UI 输出事件

### 步骤 5：将感知情绪处理收口到 `BrainController`

摄像头线程保留在 `Application`，但线程里不再直接操作：

- `session_.onEmotionDetected()`
- UI emotion emit

而是调用：

- `brainController.handlePerceptionEmotion(vr.emotion)`

### 步骤 6：加入 Brain 事件输出

例如：

- `brain.turn.started`
- `brain.turn.completed`
- `brain.emotion.changed`

这一阶段哪怕只有日志用途，也值得先加。

---

## 9. 推荐的第二阶段过渡接口

### 9.1 `BrainController` 对外接口建议

```text
class BrainController {
public:
    void init(...);
    void bindUIEvents();
    void handleUserTextInput(const std::string& text);
    void handlePerceptionEmotion(const std::string& emotion);
    bool isBusy() const;
};
```

### 9.2 `Application` 与 `BrainController` 的关系

第二阶段推荐关系：

```text
Application
 ├── 持有 ModuleRegistry
 ├── 持有 BrainController
 ├── 驱动 UI tick
 └── 驱动 perception 线程

BrainController
 ├── 使用 brain::Session
 ├── 使用 EventBus
 ├── 调用 Intelligence
 └── 输出 UI / Action 事件
```

---

## 10. 第二阶段推荐的最小代码改动原则

为了降低风险，建议严格遵守：

### 10.1 先迁文件，再迁职责

不要在同一次改动里既移动目录，又大拆逻辑，又改事件流。

### 10.2 先封装接口，再替换调用点

先让 `BrainController` 提供和原 `Application` 类似的行为入口，再一点点把 `Application` 中的旧逻辑挪过去。

### 10.3 允许短期存在“过渡重复”

例如：

- `Application` 仍知道 UI 事件
- `BrainController` 也开始接手部分事件

只要迁移是逐步收敛的，就比一次性大动稳定得多。

---

## 11. 第二阶段建议的提交拆分

建议按 5 个提交推进：

### 提交 1：引入 `brain/` 目录与空骨架

- 新增 `brain_controller.*`
- 新增 `brain_types.h`
- 新增 `brain_events.h`

### 提交 2：迁移 `session.*` 到 `brain/`

- 文件移动
- include 修正
- CMake 修正

### 提交 3：接入 `BrainController`

- `Application` 持有并初始化 BrainController

### 提交 4：迁移 UI 输入编排逻辑

- 将 `user.input.text` 流程从 `Application` 挪到 Brain

### 提交 5：迁移感知情绪编排逻辑

- 将 perception emotion 流程收口到 Brain

这样每一步都可以独立验证。

---

## 12. 第二阶段验证方案

### 12.1 编译验证

必须通过：

```bash
cmake --build build
```

### 12.2 运行验证

至少验证：

```bash
./build/ai_pet --ui
./build/ai_pet --chat
```

### 12.3 UI 模式重点验证点

1. 输入一条消息后，AI 回复是否正常出现
2. thinking 状态是否正常显示/关闭
3. 好感、关系、情绪是否仍能更新
4. 摄像头识别到表情时，宠物表情与状态栏是否正常变化
5. 退出逻辑是否正常

### 12.4 日志验证

建议增加 Brain 日志，观察这些关键节点：

- 收到用户输入
- 开始一轮 turn
- turn 完成
- 感知情绪输入
- 情绪状态变化

---

## 13. 第二阶段完成标准

满足以下条件，才算第二阶段完成：

1. `brain/` 目录已建立并投入使用
2. `Session` 已迁入 `brain/`
3. `BrainController` 已成为 Brain 对外入口
4. UI 输入对话编排已由 Brain 负责
5. 感知情绪编排已由 Brain 负责
6. `Application` 中不再直接承担主要会话决策逻辑
7. UI 与 chat 模式至少可以稳定运行

---

## 14. 第二阶段结束后的下一步

第二阶段完成后，第三阶段才建议开始：

1. 从 `brain/session.*` 中拆出：
   - `conversation_orchestrator`
   - `emotion_state`
   - `relation_state`
2. 引入系统级 `app_event_bus`
3. 准备动作层接口

所以第二阶段是：

**Brain 入场**

第三阶段才是：

**Brain 细化**

---

## 15. 结论

第二阶段最重要的不是“多拆几个类”，而是：

**把系统编排权从 `Application` 逐步转移给 Brain。**

推荐这一阶段坚持两个原则：

1. `Session` 先迁归 Brain，再考虑细拆
2. 先让 `BrainController` 接管输入与感知编排，再推进事件总线升级

一句话总结：

**第二阶段的任务，是让 Brain 从设计概念变成工程中的真实中枢。**
