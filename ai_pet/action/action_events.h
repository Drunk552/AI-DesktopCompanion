#pragma once

#include <string>

namespace action::events {

struct ChatReplyEvent {
    std::string text;
};

struct PetStateEvent {
    std::string value;
};

struct NotificationEvent {
    std::string text;
};

struct ProactiveBehaviorEvent {
    std::string text;
    std::string type;
};

struct ThinkingEvent {
    bool active = false;
};

inline constexpr const char* kChatReply = "action.chat.reply";
inline constexpr const char* kChatThinkingStart = "action.chat.thinking.start";
inline constexpr const char* kChatThinkingEnd = "action.chat.thinking.end";
inline constexpr const char* kPetEmotion = "action.pet.emotion";
inline constexpr const char* kPetAffection = "action.pet.affection";
inline constexpr const char* kPetRelationship = "action.pet.relationship";
inline constexpr const char* kNotificationShow = "action.notification.show";
inline constexpr const char* kBehaviorProactiveCare = "action.behavior.proactive_care";
inline constexpr const char* kBehaviorProactiveCheckIn = "action.behavior.proactive_check_in";
inline constexpr const char* kBehaviorProactiveTease = "action.behavior.proactive_tease";
inline constexpr const char* kBehaviorProactiveReminder = "action.behavior.proactive_reminder";

}  // namespace action::events
