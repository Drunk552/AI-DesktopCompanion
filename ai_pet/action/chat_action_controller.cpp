#include "action/chat_action_controller.h"
#include "action/action_events.h"
#include "action/behavior_policy.h"
#include "app/app_event_bus.h"

namespace action {

ChatActionController::ChatActionController(AppEventBus& eventBus, BehaviorPolicy& behaviorPolicy)
    : eventBus_(eventBus)
    , behaviorPolicy_(behaviorPolicy) {}

void ChatActionController::startThinking() {
    eventBus_.emitTyped(events::kChatThinkingStart, events::ThinkingEvent{true});
}

void ChatActionController::stopThinking() {
    eventBus_.emitTyped(events::kChatThinkingEnd, events::ThinkingEvent{false});
}

void ChatActionController::showReply(const std::string& text) {
    eventBus_.emitTyped(events::kChatReply, events::ChatReplyEvent{behaviorPolicy_.normalizeReply(text)});
}

void ChatActionController::showSystemError(const std::string& text) {
    eventBus_.emitTyped(events::kChatReply, events::ChatReplyEvent{behaviorPolicy_.normalizeReply(text)});
}

}  // namespace action
