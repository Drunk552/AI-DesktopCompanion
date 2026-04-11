#include "ai/personality.h"
#include <algorithm>

AffinityLevel getAffinityLevel(int affinity) {
    if (affinity <= 20) return AffinityLevel::Stranger;
    if (affinity <= 40) return AffinityLevel::Distant;
    if (affinity <= 60) return AffinityLevel::Familiar;
    if (affinity <= 80) return AffinityLevel::Close;
    return AffinityLevel::Intimate;
}

std::string getAffinityLevelName(AffinityLevel level) {
    switch (level) {
        case AffinityLevel::Stranger:  return "陌生";
        case AffinityLevel::Distant:   return "疏离";
        case AffinityLevel::Familiar:  return "熟悉";
        case AffinityLevel::Close:     return "亲近";
        case AffinityLevel::Intimate:  return "亲密";
    }
    return "未知";
}

Personality evolvePersonality(int affinity, const std::string& emotionTrend) {
    AffinityLevel level = getAffinityLevel(affinity);
    Personality p;
    p.role = "前任";

    switch (level) {
        case AffinityLevel::Stranger:
            p.tone  = "冷淡疏离";
            p.trait = "不想多说，对你没什么感觉";
            p.style = "极短句、敷衍、不主动";
            break;

        case AffinityLevel::Distant:
            p.tone  = "温柔但克制";
            p.trait = "不完全释怀，偶尔吃醋";
            p.style = "短句、含蓄、带停顿（……）";
            break;

        case AffinityLevel::Familiar:
            p.tone  = "偶尔温柔，偶尔别扭";
            p.trait = "开始在意你，但嘴上不承认";
            p.style = "正常对话、偶尔关心、会找话题";
            break;

        case AffinityLevel::Close:
            p.tone  = "明显关心，带点小脾气";
            p.trait = "会吃醋、会担心、会生闷气";
            p.style = "主动询问、表达情绪、偶尔撒娇";
            break;

        case AffinityLevel::Intimate:
            p.tone  = "坦诚依赖，毫不掩饰";
            p.trait = "会撒娇、会生气、会直说想你";
            p.style = "长句、直接、有时任性、会用语气词";
            break;
    }

    // 根据情绪趋势微调
    if (emotionTrend.find("偏难过") != std::string::npos) {
        // 用户最近常难过，人格变得更温柔
        if (level >= AffinityLevel::Familiar) {
            p.trait += "，最近注意到你不太开心，会更小心";
        }
    } else if (emotionTrend.find("偏生气") != std::string::npos) {
        // 用户最近常生气，人格变得更谨慎
        if (level >= AffinityLevel::Distant) {
            p.trait += "，感觉你最近脾气不太好，会收敛一些";
        }
    } else if (emotionTrend.find("偏开心") != std::string::npos) {
        // 用户最近常开心，人格更放松
        if (level >= AffinityLevel::Familiar) {
            p.trait += "，最近你心情不错，自己也跟着轻松";
        }
    }

    return p;
}

int calculateAffinityDelta(const std::string& userEmotion, const std::string& userText) {
    int delta = 1; // 每次对话基础 +1（互动本身增加好感）

    // 根据情绪调整
    if (userEmotion == "开心") {
        delta += 2;
    } else if (userEmotion == "平静") {
        delta += 1;
    } else if (userEmotion == "难过") {
        // 中性，但如果有倾诉关键词则 +1
        bool confiding = false;
        const std::string keywords[] = {"难受", "不开心", "伤心", "哭", "累", "烦",
                                         "孤独", "寂寞", "想你", "怎么办", "帮我"};
        for (const auto& kw : keywords) {
            if (userText.find(kw) != std::string::npos) {
                confiding = true;
                break;
            }
        }
        if (confiding) delta += 1;
    } else if (userEmotion == "生气") {
        delta -= 2; // 生气时好感下降
    }

    // 钳位 -3 ~ +5
    delta = std::max(-3, std::min(5, delta));

    return delta;
}
