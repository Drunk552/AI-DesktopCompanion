#pragma once

#include <string>

// 好感度等级
enum class AffinityLevel {
    Stranger,   // 0-20:  陌生、冷漠
    Distant,    // 21-40: 疏离、克制
    Familiar,   // 41-60: 熟悉、偶尔温柔
    Close,      // 61-80: 亲近、关心
    Intimate    // 81-100: 依赖、坦诚
};

// 人格定义
struct Personality {
    std::string role;
    std::string tone;
    std::string trait;
    std::string style;
};

// 根据好感度获取等级
AffinityLevel getAffinityLevel(int affinity);

// 获取等级的中文名称
std::string getAffinityLevelName(AffinityLevel level);

// 根据好感度和情绪趋势，动态生成当前人格
Personality evolvePersonality(int affinity, const std::string& emotionTrend);

// 计算好感度变化值（根据用户情绪和文本内容）
int calculateAffinityDelta(const std::string& userEmotion, const std::string& userText);
