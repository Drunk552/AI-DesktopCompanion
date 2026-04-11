#pragma once

#include <string>
#include "ai/persona_loader.h"

// 构造 Prompt：融合动态人格 + 情绪策略 + 用户输入 + 历史上下文
// 当 persona 不为空且已加载时，使用 Persona 模式（5层人格驱动）
// 否则回退到原有硬编码人格系统
std::string buildPrompt(const std::string& userInput,
                        const std::string& emotion = "平静",
                        const std::string& context = "",
                        int affinity = 30,
                        const std::string& emotionTrend = "",
                        const PersonaData* persona = nullptr);
