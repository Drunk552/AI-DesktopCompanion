#pragma once

#include "brain/brain_types.h"
#include "intelligence/memory/memory_db.h"
#include "action/strategy_state.h"
#include <chrono>
#include <string>

namespace action {

class StrategyStateService;

enum class DisturbanceLevel {
    Quiet,
    Normal,
    Clingy,
};

enum class ProactiveBehaviorType {
    Care,
    CheckIn,
    Tease,
    Reminder,
};

class BehaviorPolicy {
public:
    explicit BehaviorPolicy(StrategyStateService& strategyStateService);
    void setDisturbanceMode(const std::string& mode);
    void setPersonaProfile(const std::string& personaName);
    void recordUserInteraction();
    std::string normalizeReply(const std::string& text) const;
    bool shouldShowNotification(const std::string& text);
    bool shouldTriggerProactiveBehavior(const brain::BrainState& state);
    bool shouldTriggerIdleBehavior(const brain::BrainState& state);
    bool canTriggerIdleBehavior(const brain::BrainState& state) const;
    ProactiveBehaviorType selectProactiveBehavior(const brain::BrainState& state) const;
    ProactiveBehaviorType selectIdleBehavior(const brain::BrainState& state) const;
    std::string buildProactiveMessage(ProactiveBehaviorType type, const brain::BrainState& state) const;
    DisturbanceLevel disturbanceLevel(const brain::BrainState& state) const;
    std::string disturbanceModeLabel() const;
    std::string personaStyleLabel() const;
    std::string strategySummary(const brain::BrainState& state) const;
    std::string statusBarSummary(const brain::BrainState& state) const;
    std::string behaviorStatsSummary() const;
    std::string lastProactiveBehaviorLabel() const;
    std::string lastUserInteractionLabel() const;
    bool isQuietHourActive() const;
    void markProactiveBehavior(ProactiveBehaviorType type);
    void resetStrategyState();
    void setBehaviorEnabled(ProactiveBehaviorType type, bool enabled);
    bool isBehaviorEnabled(ProactiveBehaviorType type) const;
    void setSilentAtNight(bool enabled);
    bool silentAtNight() const;
    MemoryDB::AppStateSnapshot exportState() const;
    void importState(const MemoryDB::AppStateSnapshot& state);

private:
    std::string limitReplySentences(const std::string& text, size_t maxSentences) const;
    int proactiveCooldownSeconds(DisturbanceLevel level) const;
    bool isQuietHour() const;
    bool hasRecentInteractionBurst() const;
    bool isIdleLongEnough(DisturbanceLevel level) const;

    DisturbanceLevel configuredLevel_ = DisturbanceLevel::Normal;
    std::string personaProfile_;
    StrategyStateService& strategyStateService_;
};

}  // namespace action
