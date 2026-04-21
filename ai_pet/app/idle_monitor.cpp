#include "app/idle_monitor.h"
#include "app/app_event_bus.h"
#include "brain/brain_events.h"

IdleMonitor::~IdleMonitor() {
    stop();
}

void IdleMonitor::start(AppEventBus& eventBus, std::atomic<bool>& running, std::chrono::seconds interval) {
    stop();
    worker_ = std::thread([&eventBus, &running, interval]() {
        while (running) {
            std::this_thread::sleep_for(interval);
            if (!running) {
                break;
            }
            eventBus.emit(brain::events::kSystemIdleTimeout);
        }
    });
}

void IdleMonitor::stop() {
    if (worker_.joinable()) {
        worker_.join();
    }
}
