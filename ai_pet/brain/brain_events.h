#pragma once

#include <string>

namespace brain::events {

struct UserTextInputEvent {
    std::string text;
};

struct PerceptionEmotionEvent {
    std::string emotion;
};

struct BrainTurnCompletedEvent {
    bool accepted = false;
    bool success = false;
    std::string reply;
};

struct BrainTurnStartedEvent {};

inline constexpr const char* kUserInputText = "user.input.text";
inline constexpr const char* kPerceptionEmotionDetected = "perception.emotion.detected";
inline constexpr const char* kSystemIdleTimeout = "system.idle.timeout";
inline constexpr const char* kBrainTurnStarted = "brain.turn.started";
inline constexpr const char* kBrainTurnCompleted = "brain.turn.completed";
inline constexpr const char* kBrainEmotionChanged = "brain.emotion.changed";
inline constexpr const char* kBrainStateChanged = "brain.state.changed";

}  // namespace brain::events
