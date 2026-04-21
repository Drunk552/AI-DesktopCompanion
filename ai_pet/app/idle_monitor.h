#pragma once

#include <atomic>
#include <chrono>
#include <thread>

class AppEventBus;

class IdleMonitor {
public:
    IdleMonitor() = default;
    ~IdleMonitor();

    void start(AppEventBus& eventBus, std::atomic<bool>& running, std::chrono::seconds interval = std::chrono::seconds(15));
    void stop();

private:
    std::thread worker_;
};
