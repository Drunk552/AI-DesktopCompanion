#include "action/notification_action_controller.h"
#include "action/action_events.h"
#include "action/behavior_policy.h"
#include "app/app_event_bus.h"

namespace action {

NotificationActionController::NotificationActionController(AppEventBus& eventBus, BehaviorPolicy& behaviorPolicy)
    : eventBus_(eventBus)
    , behaviorPolicy_(behaviorPolicy) {}

void NotificationActionController::show(const std::string& text) {
    if (behaviorPolicy_.shouldShowNotification(text)) {
        eventBus_.emitTyped(events::kNotificationShow, events::NotificationEvent{text});
    }
}

}  // namespace action
