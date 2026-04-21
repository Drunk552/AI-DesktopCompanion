# AI 桌面宠物第一阶段目录迁移实施方案

## 1. 文档目标

本文用于定义 `ai_pet` 工程五层架构重组的第一阶段实施方案。

第一阶段的原则是：

1. 先做低风险目录重组
2. 不大改业务逻辑
3. 不一次性拆 `Session`
4. 保持现有功能可运行、可回归验证

这一阶段的目标不是完成整个架构升级，而是先把目录语义拉正，为后续 Brain 层和动作层落地打基础。

---

## 2. 第一阶段迁移目标

第一阶段建议只做以下目录级迁移：

### 2.1 迁移到 `app/`

- `controller/application.*` -> `app/application.*`
- `controller/module_registry.*` -> `app/module_registry.*`

### 2.2 迁移到 `perception/`

- `vision/camera.*` -> `perception/camera.*`
- `vision/face.*` -> `perception/face.*`
- `vision/emotion.*` -> `perception/emotion.*`
- `vision/vision_pipeline.*` -> `perception/vision_pipeline.*`

### 2.3 迁移到 `shared/`

- `config/config.*` -> `shared/config/config.*`
- `logger/logger.*` -> `shared/logger/logger.*`

### 2.4 本阶段暂不迁移

以下部分建议先不动：

- `ui/`
- `memory/`
- `ai/`
- `controller/session.*`

其中 `session.*` 未来会进入 `brain/`，但第一阶段不建议动它。

---

## 3. 为什么第一阶段只做这些

### 3.1 这些目录语义已经很明确

- `application/module_registry` 本来就属于应用编排层
- `vision/*` 本来就是感知层
- `config/logger` 本来就是跨层基础设施

所以这部分迁移是“改归属”，不是“改设计”。

### 3.2 这部分迁移不需要立刻重构核心逻辑

如果一开始就动：

- `Session`
- EventBus 归属
- Brain 结构

会导致目录调整和业务重构同时发生，风险过高。

### 3.3 先把编排层和感知层命名整理正确

这会让后续：

- `brain/`
- `action/`
- `intelligence/`

的引入更顺滑。

---

## 4. 第一阶段完成后的预期结构

第一阶段结束后，推荐结构应变成：

```text
ai_pet/
├── main.cpp
├── app/
│   ├── application.h
│   ├── application.cpp
│   ├── module_registry.h
│   └── module_registry.cpp
├── perception/
│   ├── camera.h
│   ├── camera.cpp
│   ├── face.h
│   ├── face.cpp
│   ├── emotion.h
│   ├── emotion.cpp
│   ├── vision_pipeline.h
│   └── vision_pipeline.cpp
├── controller/
│   ├── session.h
│   └── session.cpp
├── ai/
├── memory/
├── ui/
├── shared/
│   ├── config/
│   │   ├── config.h
│   │   └── config.cpp
│   └── logger/
│       ├── logger.h
│       └── logger.cpp
├── personas/
├── models/
└── third_party/
```

这是一个过渡结构，不是最终结构。

---

## 5. 第一阶段的实施顺序

建议严格按顺序执行，避免 include 与构建脚本同时大范围失效。

### 步骤 1：创建新目录

先创建：

- `app/`
- `perception/`
- `shared/config/`
- `shared/logger/`

这一步不改代码逻辑，只准备目录。

### 步骤 2：迁移 `config` 与 `logger`

先迁基础设施层，原因是：

- 依赖面小
- 风险低
- 最容易回归验证

迁移后需要同步修改：

- `#include "config/config.h"` -> `#include "shared/config/config.h"`
- `#include "logger/logger.h"` -> `#include "shared/logger/logger.h"`

### 步骤 3：迁移 `vision` 到 `perception`

迁移文件后修改：

- `#include "vision/camera.h"` -> `#include "perception/camera.h"`
- `#include "vision/face.h"` -> `#include "perception/face.h"`
- `#include "vision/emotion.h"` -> `#include "perception/emotion.h"`
- `#include "vision/vision_pipeline.h"` -> `#include "perception/vision_pipeline.h"`

### 步骤 4：迁移 `application` 与 `module_registry`

迁移到 `app/` 后修改：

- `#include "controller/application.h"` -> `#include "app/application.h"`
- `#include "controller/module_registry.h"` -> `#include "app/module_registry.h"`

### 步骤 5：更新 CMakeLists.txt

将源文件路径调整到新目录。

这一阶段建议只改文件路径，不做复杂构建重构。

### 步骤 6：全量编译与 UI 启动验证

至少验证：

1. `cmake --build build`
2. `./build/ai_pet --ui`
3. `./build/ai_pet --chat`

如果其中任一失败，先修 include 与 CMake，再继续。

---

## 6. 第一阶段中每类改动的范围

### 6.1 文件移动

这类改动包括：

- 头文件路径变化
- 源文件路径变化
- CMake 源文件列表变化

### 6.2 include 路径修正

重点影响文件：

- `main.cpp`
- `app/application.*`
- `app/module_registry.*`
- `controller/session.*`
- `ui/*`
- `perception/*`

### 6.3 不应在第一阶段做的改动

以下内容建议不要夹带到第一阶段：

- 把 `Session` 拆成多个类
- 把 EventBus 从 UI 中移出
- 调整 UI 页面组织方式
- 重写线程模型
- 改 AI 调用流程

第一阶段只做目录和 include 归位。

---

## 7. 第一阶段推荐提交拆分

为了便于回滚和排查，建议不要把所有目录迁移压成一个提交。

建议分成 4 个提交：

### 提交 1：迁移 shared 基础设施

- `config/` -> `shared/config/`
- `logger/` -> `shared/logger/`
- 修正 include

### 提交 2：迁移感知层

- `vision/` -> `perception/`
- 修正 include

### 提交 3：迁移应用层

- `controller/application.*` -> `app/`
- `controller/module_registry.*` -> `app/`
- 修正 include

### 提交 4：清理构建与文档

- 更新 `CMakeLists.txt`
- 更新开发文档和目录文档

这样每一步都能独立验证。

---

## 8. 第一阶段的验证清单

### 8.1 编译验证

必须通过：

```bash
cmake --build build
```

### 8.2 运行验证

至少验证以下模式：

```bash
./build/ai_pet --ui
./build/ai_pet --chat
./build/ai_pet --camera
```

如果 `--camera` 环境依赖受限，也至少验证：

- UI 能启动
- chat 能进入主循环

### 8.3 回归验证点

重点检查：

1. UI 是否仍能启动
2. 摄像头线程是否还能正常启动
3. `Session` 是否还能读取感知层结果
4. `ModuleRegistry` 是否还能正确装配模块
5. 日志输出是否正常

---

## 9. 第一阶段的风险点

### 9.1 include 路径遗漏

这是第一阶段最常见的问题。

解决方式：

- 先迁一类目录
- 编译
- 再迁下一类

不要全部迁完再一起修。

### 9.2 CMake 源文件路径忘记更新

表现通常是：

- 编译找不到源文件
- 链接阶段缺符号

### 9.3 文档和真实目录不同步

迁移后要同步更新：

- `docs/目录结构.md`
- 开发文档中涉及旧路径的内容

### 9.4 IDE/编译数据库缓存问题

迁移目录后建议重新生成：

```bash
cmake -S . -B build
```

避免 `compile_commands.json` 仍引用旧路径。

---

## 10. 第一阶段完成标准

满足以下条件，才算第一阶段完成：

1. `app/`、`perception/`、`shared/` 已建立并投入使用
2. `application/module_registry` 已迁入 `app/`
3. `vision/*` 已迁入 `perception/`
4. `config/logger` 已迁入 `shared/`
5. `controller/session.*` 暂时保留不动
6. UI、chat 至少两个模式可正常运行
7. 构建脚本和文档已同步

---

## 11. 第一阶段之后的下一步

第一阶段完成后，第二阶段才建议开始：

1. 将 `controller/session.*` 迁入 `brain/` 作为过渡
2. 引入系统级 `app_event_bus`
3. 逐步把 `Session` 拆为：
   - `conversation_orchestrator`
   - `emotion_state`
   - `relation_state`

也就是说：

**第一阶段是“目录归位”，第二阶段才是“Brain 落地”。**

---

## 12. 结论

第一阶段应坚持一个原则：

**只改归属，不改核心业务逻辑。**

推荐本阶段只迁移：

- `application/module_registry` -> `app/`
- `vision/*` -> `perception/`
- `config/logger` -> `shared/`

并暂时保留：

- `Session`
- `UI`
- `AI`
- `Memory`

这样可以用最小风险把工程的目录语义先拉正，为后续真正的 Brain 层和动作层演进做好准备。
