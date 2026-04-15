#include "ai/prompt.h"
#include "ai/fallback_personality.h"

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

std::string buildPrompt(const PromptContext& ctx) {

    if (ctx.persona != nullptr && ctx.persona->loaded) {
        AffinityLevel level = getAffinityLevel(ctx.affinity);

        std::string prompt;

        prompt += ctx.persona->personaContent + "\n\n";

        if (!ctx.persona->memoriesContent.empty()) {
            prompt += "---\n\n";
            prompt += ctx.persona->memoriesContent + "\n\n";
        }

        prompt += "---\n\n";

        prompt += "当前与用户的关系好感度：" + std::to_string(ctx.affinity) +
                  "/100（" + getAffinityLevelName(level) + "）\n";
        prompt += getAffinityHint(ctx.affinity) + "\n\n";

        if (!ctx.emotionTrend.empty() && ctx.emotionTrend != "无情绪数据") {
            prompt += "用户" + ctx.emotionTrend + "\n\n";
        }

        if (!ctx.context.empty()) {
            prompt += ctx.context + "\n\n";
        }

        prompt +=
            "用户当前情绪：" + ctx.emotion + "\n"
            "用户说：" + ctx.userInput + "\n"
            "你：";

        return prompt;
    }

    FallbackPersona p = evolveFallbackPersona(ctx.affinity, ctx.emotionTrend);
    AffinityLevel level = getAffinityLevel(ctx.affinity);
    std::string strategy = getFallbackStrategy(ctx.emotion, level);

    std::string prompt =
        "你是用户的" + p.role + "。\n"
        "性格：" + p.tone + "\n"
        "特点：" + p.trait + "\n\n"

        "当前与用户的关系等级：" + getAffinityLevelName(level) +
        "（好感度 " + std::to_string(ctx.affinity) + "/100）\n"
        "当前回应策略：" + strategy + "\n\n"

        "表达风格要求：\n"
        "- " + p.style + "\n"
        "- 避免直接表达爱\n"
        "- 允许停顿（……）\n"
        "- 偶尔反问\n"
        "- 不解释太多\n"
        "- 回复控制在 1~3 句话\n\n";

    if (!ctx.emotionTrend.empty() && ctx.emotionTrend != "无情绪数据") {
        prompt += "用户" + ctx.emotionTrend + "\n";
    }

    if (!ctx.context.empty()) {
        prompt += ctx.context + "\n";
    }

    prompt +=
        "用户当前情绪：" + ctx.emotion + "\n"
        "用户说：" + ctx.userInput + "\n"
        "你：";

    return prompt;
}
