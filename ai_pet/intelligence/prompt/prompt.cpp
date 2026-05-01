#include "intelligence/prompt/prompt.h"
#include "intelligence/persona/fallback_personality.h"
#include <algorithm>
#include <cctype>

namespace {

std::string to_lower_ascii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

bool contains_any(const std::string& text, const std::initializer_list<const char*> keywords) {
    for (const char* keyword : keywords) {
        if (text.find(keyword) != std::string::npos) {
            return true;
        }
    }
    return false;
}

bool is_self_intro_request(const std::string& userInput) {
    const std::string lowered = to_lower_ascii(userInput);
    return contains_any(userInput, {
               "介绍一下你自己",
               "简单介绍一下你自己",
               "请简单介绍一下你自己",
               "介绍你自己",
               "你是谁",
               "说说你自己",
               "自我介绍"
           }) ||
           contains_any(lowered, {
               "introduce yourself",
               "briefly introduce yourself",
               "please briefly introduce yourself",
               "tell me about yourself",
               "who are you"
           });
}

std::string clip_text(const std::string& text, size_t maxLen) {
    if (text.size() <= maxLen) {
        return text;
    }
    return text.substr(0, maxLen) + "\n...";
}

bool is_direct_question_request(const std::string& userInput) {
    const std::string lowered = to_lower_ascii(userInput);
    if (contains_any(userInput, {
            "吗",
            "？",
            "今天休息",
            "在干嘛",
            "做什么",
            "吃了什么",
            "忙吗",
            "上班吗",
            "下班了吗"
        })) {
        return true;
    }

    return contains_any(lowered, {
        "?",
        "are you ",
        "do you ",
        "did you ",
        "what ",
        "when ",
        "where ",
        "how ",
        "have you ",
        "will you "
    });
}

std::string build_self_intro_prompt(const PromptContext& ctx) {
    std::string prompt;
    prompt +=
        "你现在只需要完成一个任务：用户在请求你做简短自我介绍。\n"
        "硬性要求：\n"
        "1. 直接回答用户的问题，不要评论用户的表情、眼神、停顿、状态、语气，也不要描写动作或舞台感。\n"
        "2. 不要先铺垫，不要反问，不要把回答引到别的话题上。\n"
        "3. 用第一人称做自我介绍，内容以当前 persona 身份为准。\n"
        "4. 回复控制在 2 到 4 句，简洁、自然、像正常聊天。\n"
        "5. 无论用户使用中文还是英文提问，都必须只用中文回答；不要输出英文句子，不要中英混合。\n\n";

    if (ctx.persona != nullptr && ctx.persona->loaded) {
        prompt += "当前 persona 名称：" + ctx.persona->name + "\n";
        prompt += "下面是角色设定，请只提炼最基本的身份、特点和交流风格，不要戏剧化扩写：\n";
        prompt += ctx.persona->personaContent + "\n\n";
    } else {
        const FallbackPersona p = evolveFallbackPersona(ctx.affinity, ctx.emotionTrend);
        prompt +=
            "当前没有显式 persona，请按以下角色设定进行简短自我介绍：\n"
            "身份：" + p.role + "\n"
            "性格：" + p.tone + "\n"
            "特点：" + p.trait + "\n"
            "风格：" + p.style + "\n\n";
    }

    prompt += "用户输入：" + ctx.userInput + "\n";
    prompt += "你的回答：";
    return prompt;
}

std::string build_direct_question_prompt(const PromptContext& ctx) {
    std::string prompt;
    prompt +=
        "你现在只需要回答用户这一句直接问题。\n"
        "硬性要求：\n"
        "1. 先直接回答当前问题，不要点评上一轮对话，不要分析用户，不要复述上下文。\n"
        "2. 回复控制在 1 到 3 句，简洁自然，像正常聊天。\n"
        "3. 如果问题本质上是 yes/no 问句，先明确回答 yes/no 对应的自然表达，再补一句说明。\n"
        "4. 无论用户当前输入是中文还是英文，回复都必须使用自然中文；如果用户用英文提问，只做语义理解，不要跟着输出英文。\n"
        "5. 不要输出舞台描写、心理描写、动作描写、旁白、点评。\n\n";

    if (ctx.persona != nullptr && ctx.persona->loaded) {
        prompt += "当前 persona 名称：" + ctx.persona->name + "\n";
        prompt += "角色设定摘要：\n";
        prompt += clip_text(ctx.persona->personaContent, 1200) + "\n\n";
    } else {
        const FallbackPersona p = evolveFallbackPersona(ctx.affinity, ctx.emotionTrend);
        prompt +=
            "当前角色摘要：\n"
            "身份：" + p.role + "\n"
            "性格：" + p.tone + "\n"
            "特点：" + p.trait + "\n"
            "风格：" + p.style + "\n\n";
    }

    prompt += "用户输入：" + ctx.userInput + "\n";
    prompt += "回答：";
    return prompt;
}

}  // namespace

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
    if (is_self_intro_request(ctx.userInput)) {
        return build_self_intro_prompt(ctx);
    }

    if (is_direct_question_request(ctx.userInput)) {
        return build_direct_question_prompt(ctx);
    }

    const std::string turnRules =
        "当前回合回答规则（高于下面的背景信息）：\n"
        "1. 必须先直接回应用户这一句最新输入，不能忽略、岔开或续写上一轮话题。\n"
        "2. 只有当历史对话、共同记忆、情绪趋势与当前问题直接相关时，才能引用；不相关就不要提。\n"
        "3. 不要把系统主动问候、状态播报、安慰话术当成当前问题的答案。\n"
        "4. 如果用户在问“你是谁”“介绍一下你自己”“你是做什么的”这类问题，就直接按当前 persona 身份做自我介绍。\n"
        "5. 不要评价、复述、点评上一轮回答的风格、张力、质量或效果，除非用户明确要求你这么做。\n"
        "6. 如果用户这一句是直接问题，比如在问今天是否休息、在做什么、最近忙不忙，就先给直接答案，再补一小句自然说明。\n"
        "7. 回复语言固定为中文：即使用户这一句使用英文提问，也只允许输出中文，不要被用户输入语言带偏。\n"
        "8. 回复只输出你这次要发给用户的话，不要输出分析过程，不要复述规则。\n\n";

    if (ctx.persona != nullptr && ctx.persona->loaded) {
        AffinityLevel level = getAffinityLevel(ctx.affinity);

        std::string prompt;

        prompt += turnRules;

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
            "=== 当前回合 ===\n"
            "用户当前情绪：" + ctx.emotion + "\n"
            "用户最新输入：" + ctx.userInput + "\n"
            "本轮输出语言：中文\n"
            "请先直接回答这句最新输入，再决定是否少量引用上面的背景。\n"
            "你：";

        return prompt;
    }

    FallbackPersona p = evolveFallbackPersona(ctx.affinity, ctx.emotionTrend);
    AffinityLevel level = getAffinityLevel(ctx.affinity);
    std::string strategy = getFallbackStrategy(ctx.emotion, level);

    std::string prompt =
        turnRules +
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
        "=== 当前回合 ===\n"
        "用户当前情绪：" + ctx.emotion + "\n"
        "用户最新输入：" + ctx.userInput + "\n"
        "本轮输出语言：中文\n"
        "请先直接回答这句最新输入，再决定是否少量引用上面的背景。\n"
        "你：";

    return prompt;
}
