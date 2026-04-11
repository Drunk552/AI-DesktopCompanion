#include "ai/prompt.h"
#include "ai/personality.h"
#include "ai/strategy.h"

// 好感度微调提示（仅 Persona 模式使用）
static std::string getAffinityHint(int affinity) {
    AffinityLevel level = getAffinityLevel(affinity);
    switch (level) {
        case AffinityLevel::Stranger:
            return "好感度很低，请严格遵守 Layer 0 中的冷淡/疏离规则，回复极简";
        case AffinityLevel::Distant:
            return "好感度偏低，保持克制，不要太主动";
        case AffinityLevel::Familiar:
            return "好感度适中，可以自然交流";
        case AffinityLevel::Close:
            return "好感度较高，可以更主动和关心";
        case AffinityLevel::Intimate:
            return "好感度很高，可以完全按照 Layer 0 中亲密行为表现";
    }
    return "";
}

std::string buildPrompt(const std::string& userInput,
                        const std::string& emotion,
                        const std::string& context,
                        int affinity,
                        const std::string& emotionTrend,
                        const PersonaData* persona) {

    // === Persona 模式：使用加载的角色文件 ===
    if (persona != nullptr && persona->loaded) {
        AffinityLevel level = getAffinityLevel(affinity);

        std::string prompt;

        // persona.md 全文作为系统提示词核心
        prompt += persona->personaContent + "\n\n";

        // memories.md 全文作为记忆上下文
        if (!persona->memoriesContent.empty()) {
            prompt += "---\n\n";
            prompt += persona->memoriesContent + "\n\n";
        }

        prompt += "---\n\n";

        // 好感度微调提示
        prompt += "当前与用户的关系好感度：" + std::to_string(affinity) +
                  "/100（" + getAffinityLevelName(level) + "）\n";
        prompt += getAffinityHint(affinity) + "\n\n";

        // 注入情绪趋势
        if (!emotionTrend.empty() && emotionTrend != "无情绪数据") {
            prompt += "用户" + emotionTrend + "\n\n";
        }

        // 注入历史上下文
        if (!context.empty()) {
            prompt += context + "\n\n";
        }

        prompt +=
            "用户当前情绪：" + emotion + "\n"
            "用户说：" + userInput + "\n"
            "你：";

        return prompt;
    }

    // === 回退模式：原有硬编码人格系统 ===
    Personality p = evolvePersonality(affinity, emotionTrend);
    AffinityLevel level = getAffinityLevel(affinity);
    std::string strategy = getStrategy(emotion, level);

    std::string prompt =
        "你是用户的" + p.role + "。\n"
        "性格：" + p.tone + "\n"
        "特点：" + p.trait + "\n\n"

        "当前与用户的关系等级：" + getAffinityLevelName(level) +
        "（好感度 " + std::to_string(affinity) + "/100）\n"
        "当前回应策略：" + strategy + "\n\n"

        "表达风格要求：\n"
        "- " + p.style + "\n"
        "- 避免直接表达爱\n"
        "- 允许停顿（……）\n"
        "- 偶尔反问\n"
        "- 不解释太多\n"
        "- 回复控制在 1~3 句话\n\n";

    if (!emotionTrend.empty() && emotionTrend != "无情绪数据") {
        prompt += "用户" + emotionTrend + "\n";
    }

    if (!context.empty()) {
        prompt += context + "\n";
    }

    prompt +=
        "用户当前情绪：" + emotion + "\n"
        "用户说：" + userInput + "\n"
        "你：";

    return prompt;
}
