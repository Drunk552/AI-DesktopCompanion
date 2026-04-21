#pragma once

#include <functional>
#include <string>

class AppEventBus;

namespace brain {

struct BrainEventHandlers {
    std::function<void(const std::string&)> onUserTextInput;
    std::function<void(const std::string&)> onPerceptionEmotion;
    std::function<void()> onSystemIdleTimeout;
};

class EventRouter {
public:
    void bind(AppEventBus& eventBus, const BrainEventHandlers& handlers);

private:
    bool bound_ = false;
};

}  // namespace brain
