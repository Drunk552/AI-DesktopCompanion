#pragma once

#include <string>

namespace brain {

struct BrainInput {
    std::string text;
    std::string source = "user";
};

struct BrainOutput {
    std::string reply;
    std::string emotion;
    int affinity = 30;
    std::string relationship;
    bool accepted = false;
    bool success = false;
};

struct SessionTurnResult {
    bool accepted = false;
    bool success = false;
    std::string reply;
    std::string emotion;
    int affinity = 30;
    std::string affinityLevel;
};

using BrainTurnResult = SessionTurnResult;

struct BrainState {
    std::string emotion = "平静";
    int affinity = 30;
    std::string relationship = "疏离";
    bool busy = false;
    std::string personaName = "未命名";
    std::string lastProactiveBehavior = "none";
    std::string lastUserInteractionAt = "never";
    std::string lastDetectedEmotion = "平静";
    std::string recentEmotionTrend = "无情绪数据";
    std::string disturbanceMode = "normal";
    std::string personaStyle = "默认风格";
    std::string activeMode = "unknown";
    bool quietHourActive = false;
    bool proactiveReady = false;
};

struct BrainStateChangedEvent {
    BrainState state;
};

}  // namespace brain
