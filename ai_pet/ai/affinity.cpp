#include "ai/affinity.h"
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

int calculateAffinityDelta(const std::string& userEmotion, const std::string& userText) {
    int delta = 1;

    if (userEmotion == "开心") {
        delta += 2;
    } else if (userEmotion == "平静") {
        delta += 1;
    } else if (userEmotion == "难过") {
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
        delta -= 2;
    }

    delta = std::max(-3, std::min(5, delta));
    return delta;
}
