#pragma once

#include <string>

class AppEventBus;
namespace action { class BehaviorPolicy; }

namespace action {

class NotificationActionController {
public:
    NotificationActionController(AppEventBus& eventBus, BehaviorPolicy& behaviorPolicy);

    void show(const std::string& text);

private:
    AppEventBus& eventBus_;
    BehaviorPolicy& behaviorPolicy_;
};

}  // namespace action
