#include "app/app_event_bus.h"

void AppEventBus::subscribe(const std::string& eventType, Callback callback) {
    subscribeAny(eventType, [callback = std::move(callback)](const std::any& payload) {
        if (const std::string* text = std::any_cast<std::string>(&payload)) {
            callback(*text);
            return;
        }
        callback(std::string());
    });
}

void AppEventBus::emit(const std::string& eventType, const std::string& data) {
    emitAny(eventType, std::any(data));
}

void AppEventBus::subscribeAny(const std::string& eventType, AnyCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    listeners_[eventType].push_back(std::move(callback));
}

void AppEventBus::emitAny(const std::string& eventType, const std::any& data) {
    std::vector<AnyCallback> callbacks;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = listeners_.find(eventType);
        if (it == listeners_.end()) {
            return;
        }
        callbacks = it->second;
    }

    for (const auto& callback : callbacks) {
        callback(data);
    }
}
