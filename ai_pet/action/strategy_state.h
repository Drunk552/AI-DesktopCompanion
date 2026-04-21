#pragma once

#include "intelligence/memory/memory_db.h"
#include <chrono>
#include <deque>
#include <string>

namespace action {

struct BehaviorStats {
    int careCount = 0;
    int checkInCount = 0;
    int teaseCount = 0;
    int reminderCount = 0;
};

struct StrategyState {
    std::string lastNotification;
    std::chrono::steady_clock::time_point lastNotificationAt{};
    std::chrono::steady_clock::time_point lastProactiveAt{};
    std::chrono::steady_clock::time_point lastUserInteractionAt{};
    std::deque<std::chrono::steady_clock::time_point> recentUserInteractions;
    std::string lastProactiveBehavior = "主动关怀";
    BehaviorStats stats;
    bool allowCare = true;
    bool allowCheckIn = true;
    bool allowTease = true;
    bool allowReminder = true;
    bool silentAtNight = false;
};

}  // namespace action
