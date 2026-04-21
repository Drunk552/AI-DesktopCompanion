#pragma once

#include <any>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class AppEventBus {
public:
    using Callback = std::function<void(const std::string&)>;
    using AnyCallback = std::function<void(const std::any&)>;

    void subscribe(const std::string& eventType, Callback callback);
    void emit(const std::string& eventType, const std::string& data = std::string());

    template <typename T>
    void subscribeTyped(const std::string& eventType, std::function<void(const T&)> callback) {
        subscribeAny(eventType, [callback = std::move(callback)](const std::any& payload) {
            if (const T* typed = std::any_cast<T>(&payload)) {
                callback(*typed);
            }
        });
    }

    template <typename T>
    void emitTyped(const std::string& eventType, const T& data) {
        emitAny(eventType, std::any(data));
    }

private:
    void subscribeAny(const std::string& eventType, AnyCallback callback);
    void emitAny(const std::string& eventType, const std::any& data);

    std::mutex mutex_;
    std::unordered_map<std::string, std::vector<AnyCallback>> listeners_;
};
