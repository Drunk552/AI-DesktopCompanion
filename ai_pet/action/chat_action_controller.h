#pragma once

#include <string>

class AppEventBus;
namespace action { class BehaviorPolicy; }

namespace action {

class ChatActionController {
public:
    ChatActionController(AppEventBus& eventBus, BehaviorPolicy& behaviorPolicy);

    void startThinking();
    void stopThinking();
    void showReply(const std::string& text);
    void showSystemError(const std::string& text);

private:
    AppEventBus& eventBus_;
    BehaviorPolicy& behaviorPolicy_;
};

}  // namespace action
