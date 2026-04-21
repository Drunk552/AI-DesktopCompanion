#include "brain/event_router.h"
#include "app/app_event_bus.h"
#include "brain/brain_events.h"

namespace brain {

void EventRouter::bind(AppEventBus& eventBus, const BrainEventHandlers& handlers) {
    if (bound_) {
        return;
    }

    if (handlers.onUserTextInput) {
        eventBus.subscribeTyped<events::UserTextInputEvent>(events::kUserInputText, [handler = handlers.onUserTextInput](const events::UserTextInputEvent& event) {
            handler(event.text);
        });
    }
    if (handlers.onPerceptionEmotion) {
        eventBus.subscribeTyped<events::PerceptionEmotionEvent>(events::kPerceptionEmotionDetected, [handler = handlers.onPerceptionEmotion](const events::PerceptionEmotionEvent& event) {
            handler(event.emotion);
        });
    }
    if (handlers.onSystemIdleTimeout) {
        eventBus.subscribe(events::kSystemIdleTimeout, [handler = handlers.onSystemIdleTimeout](const std::string&) {
            handler();
        });
    }

    bound_ = true;
}

}  // namespace brain
