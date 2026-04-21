#include "action/behavior_policy.h"
#include "action/strategy_state_service.h"
#include <ctime>
#include <sstream>

namespace {

long long now_epoch_seconds() {
    return static_cast<long long>(std::time(nullptr));
}

std::chrono::steady_clock::time_point restore_steady_from_epoch(long long epochSeconds) {
    if (epochSeconds <= 0) {
        return {};
    }
    const long long diff = now_epoch_seconds() - epochSeconds;
    return std::chrono::steady_clock::now() - std::chrono::seconds(diff > 0 ? diff : 0);
}

}  // namespace

namespace action {

BehaviorPolicy::BehaviorPolicy(StrategyStateService& strategyStateService)
    : strategyStateService_(strategyStateService) {}

void BehaviorPolicy::setDisturbanceMode(const std::string& mode) {
    if (mode == "quiet") {
        configuredLevel_ = DisturbanceLevel::Quiet;
        return;
    }
    if (mode == "clingy") {
        configuredLevel_ = DisturbanceLevel::Clingy;
        return;
    }
    configuredLevel_ = DisturbanceLevel::Normal;
}

void BehaviorPolicy::setPersonaProfile(const std::string& personaName) {
    personaProfile_ = personaName;
}

void BehaviorPolicy::recordUserInteraction() {
    const auto now = std::chrono::steady_clock::now();
    auto& state = strategyStateService_.state();
    state.lastUserInteractionAt = now;
    state.recentUserInteractions.push_back(now);

    while (!state.recentUserInteractions.empty() &&
           std::chrono::duration_cast<std::chrono::minutes>(now - state.recentUserInteractions.front()).count() >= 10) {
        state.recentUserInteractions.pop_front();
    }
}

std::string BehaviorPolicy::normalizeReply(const std::string& text) const {
    if (text.empty()) {
        return "……";
    }
    return limitReplySentences(text, 3);
}

bool BehaviorPolicy::shouldShowNotification(const std::string& text) {
    auto& state = strategyStateService_.state();
    if (text.empty()) {
        return false;
    }

    const auto now = std::chrono::steady_clock::now();
    if (text == state.lastNotification &&
        std::chrono::duration_cast<std::chrono::milliseconds>(now - state.lastNotificationAt).count() < 2500) {
        return false;
    }

    state.lastNotification = text;
    state.lastNotificationAt = now;
    return true;
}

bool BehaviorPolicy::shouldTriggerProactiveBehavior(const brain::BrainState& state) {
    auto& strategy = strategyStateService_.state();
    if (state.busy) {
        return false;
    }

    const DisturbanceLevel level = disturbanceLevel(state);
    if (isQuietHour() && (level == DisturbanceLevel::Quiet || strategy.silentAtNight)) {
        return false;
    }
    if (hasRecentInteractionBurst()) {
        return false;
    }
    if (!isIdleLongEnough(level)) {
        return false;
    }

    const bool isLowMood = state.emotion.find("sad") != std::string::npos ||
        state.emotion.find("难过") != std::string::npos ||
        state.emotion.find("疲") != std::string::npos ||
        state.emotion.find("累") != std::string::npos;
    if (!isLowMood) {
        return false;
    }

    const ProactiveBehaviorType type = selectProactiveBehavior(state);
    if (!isBehaviorEnabled(type)) {
        return false;
    }

    const auto now = std::chrono::steady_clock::now();
    const int cooldown = proactiveCooldownSeconds(level);
    if (strategy.lastProactiveAt.time_since_epoch().count() != 0 &&
        std::chrono::duration_cast<std::chrono::seconds>(now - strategy.lastProactiveAt).count() < cooldown) {
        return false;
    }

    strategy.lastProactiveAt = now;
    return true;
}

bool BehaviorPolicy::shouldTriggerIdleBehavior(const brain::BrainState& state) {
    auto& strategy = strategyStateService_.state();
    if (state.busy) {
        return false;
    }

    const DisturbanceLevel level = disturbanceLevel(state);
    if (isQuietHour() && (level == DisturbanceLevel::Quiet || strategy.silentAtNight)) {
        return false;
    }
    if (hasRecentInteractionBurst()) {
        return false;
    }
    if (!isIdleLongEnough(level)) {
        return false;
    }

    const ProactiveBehaviorType type = selectIdleBehavior(state);
    if (!isBehaviorEnabled(type)) {
        return false;
    }

    const auto now = std::chrono::steady_clock::now();
    const int cooldown = proactiveCooldownSeconds(level);
    if (strategy.lastProactiveAt.time_since_epoch().count() != 0 &&
        std::chrono::duration_cast<std::chrono::seconds>(now - strategy.lastProactiveAt).count() < cooldown) {
        return false;
    }

    strategy.lastProactiveAt = now;
    return true;
}

bool BehaviorPolicy::canTriggerIdleBehavior(const brain::BrainState& state) const {
    const auto& strategy = strategyStateService_.state();
    if (state.busy) {
        return false;
    }

    const DisturbanceLevel level = disturbanceLevel(state);
    if (isQuietHour() && (level == DisturbanceLevel::Quiet || strategy.silentAtNight)) {
        return false;
    }
    if (hasRecentInteractionBurst()) {
        return false;
    }
    if (!isIdleLongEnough(level)) {
        return false;
    }

    const ProactiveBehaviorType type = selectIdleBehavior(state);
    if (!isBehaviorEnabled(type)) {
        return false;
    }

    const auto now = std::chrono::steady_clock::now();
    const int cooldown = proactiveCooldownSeconds(level);
    if (strategy.lastProactiveAt.time_since_epoch().count() != 0 &&
        std::chrono::duration_cast<std::chrono::seconds>(now - strategy.lastProactiveAt).count() < cooldown) {
        return false;
    }

    return true;
}

ProactiveBehaviorType BehaviorPolicy::selectProactiveBehavior(const brain::BrainState& state) const {
    const bool isTired = state.emotion.find("疲") != std::string::npos || state.emotion.find("累") != std::string::npos;
    const bool isSad = state.emotion.find("sad") != std::string::npos || state.emotion.find("难过") != std::string::npos;
    const DisturbanceLevel level = disturbanceLevel(state);

    if (isTired) {
        return ProactiveBehaviorType::Reminder;
    }
    if (state.affinity >= 85 && level == DisturbanceLevel::Clingy && !isSad) {
        return ProactiveBehaviorType::Tease;
    }
    if (state.affinity >= 35 && !isSad) {
        return ProactiveBehaviorType::CheckIn;
    }
    return ProactiveBehaviorType::Care;
}

ProactiveBehaviorType BehaviorPolicy::selectIdleBehavior(const brain::BrainState& state) const {
    const bool isTired = state.emotion.find("疲") != std::string::npos || state.emotion.find("累") != std::string::npos;
    const bool isSad = state.emotion.find("sad") != std::string::npos || state.emotion.find("难过") != std::string::npos;
    const DisturbanceLevel level = disturbanceLevel(state);

    if (isTired) {
        return ProactiveBehaviorType::Reminder;
    }
    if (isSad) {
        return ProactiveBehaviorType::Care;
    }
    if (state.affinity >= 85 && level == DisturbanceLevel::Clingy && !isSad) {
        return ProactiveBehaviorType::Tease;
    }
    return ProactiveBehaviorType::CheckIn;
}

std::string BehaviorPolicy::buildProactiveMessage(ProactiveBehaviorType type, const brain::BrainState& state) const {
    const DisturbanceLevel level = disturbanceLevel(state);
    const bool isPrincessStyle = personaProfile_.find("公主") != std::string::npos;
    const bool isWarmRelation = state.relationship.find("亲") != std::string::npos ||
        state.relationship.find("依") != std::string::npos ||
        state.affinity >= 60;

    if (type == ProactiveBehaviorType::Reminder) {
        if (isPrincessStyle) {
            return state.affinity >= 60 ? "先去休息，别让我重复第二遍。" : "……你该休息了，别硬撑。";
        }
        return state.affinity >= 60 ? "你已经很累了，先停一下，我不想看你继续硬撑。" : "先休息一下吧，别把自己拖垮。";
    }

    if (type == ProactiveBehaviorType::Tease) {
        if (isPrincessStyle) {
            return "又在偷偷发呆？被我抓到了吧。";
        }
        return "你这副样子，我可不能当作没看见。";
    }

    if (type == ProactiveBehaviorType::CheckIn) {
        if (isPrincessStyle) {
            return isWarmRelation ? "怎么突然安静下来了，要不要和我说说？" : "你今天有点安静，是发生什么了吗？";
        }
        return isWarmRelation ? "我感觉你今天不太对，要不要聊两句？" : "你今天状态有点怪，要不要缓一缓？";
    }

    if (isPrincessStyle) {
        if (isWarmRelation) {
            return level == DisturbanceLevel::Clingy
                ? "别一个人闷着了，我陪着你，这次不许躲开。"
                : "你状态不太好，我会在这儿陪着你。";
        }
        return level == DisturbanceLevel::Quiet
            ? "……先去休息吧，别硬撑。"
            : "你今天脸色不太好，先照顾好自己。";
    }

    if (state.affinity >= 60) {
        if (level == DisturbanceLevel::Clingy) {
            return "你看起来不太开心，我会一直陪着你。";
        }
        return "你看起来不太开心，我会陪着你。";
    }
    if (state.affinity >= 30) {
        if (level == DisturbanceLevel::Quiet) {
            return "你今天状态不太对，先休息一下也好。";
        }
        return "你今天状态不太对，要不要先休息一下？";
    }
    return "……如果你不舒服，就先别勉强自己。";
}

DisturbanceLevel BehaviorPolicy::disturbanceLevel(const brain::BrainState& state) const {
    if (configuredLevel_ == DisturbanceLevel::Quiet) {
        return DisturbanceLevel::Quiet;
    }
    if (configuredLevel_ == DisturbanceLevel::Clingy) {
        return DisturbanceLevel::Clingy;
    }
    if (state.affinity >= 70) {
        return DisturbanceLevel::Clingy;
    }
    if (state.affinity >= 35) {
        return DisturbanceLevel::Normal;
    }
    return DisturbanceLevel::Quiet;
}

int BehaviorPolicy::proactiveCooldownSeconds(DisturbanceLevel level) const {
    switch (level) {
        case DisturbanceLevel::Quiet:
            return 180;
        case DisturbanceLevel::Normal:
            return 90;
        case DisturbanceLevel::Clingy:
            return 60;
    }
    return 90;
}

std::string BehaviorPolicy::limitReplySentences(const std::string& text, size_t maxSentences) const {
    size_t sentenceCount = 0;
    size_t cutPos = std::string::npos;

    for (size_t i = 0; i < text.size(); ++i) {
        const char c = text[i];
        bool isSentenceEnd = c == '\n' || c == '!' || c == '?' || c == '.';
        if (!isSentenceEnd && i + 2 < text.size()) {
            const std::string punct = text.substr(i, 3);
            isSentenceEnd = punct == "。" || punct == "！" || punct == "？";
        }
        if (!isSentenceEnd) {
            continue;
        }

        ++sentenceCount;
        if (sentenceCount >= maxSentences) {
            cutPos = (c == '\n' || c == '!' || c == '?' || c == '.') ? (i + 1) : (i + 3);
            break;
        }
    }

    if (cutPos == std::string::npos) {
        return text;
    }

    std::string limited = text.substr(0, cutPos);
    while (!limited.empty() && (limited.back() == '\n' || limited.back() == ' ' || limited.back() == '\t')) {
        limited.pop_back();
    }
    return limited.empty() ? "……" : limited;
}

std::string BehaviorPolicy::disturbanceModeLabel() const {
    switch (configuredLevel_) {
        case DisturbanceLevel::Quiet:
            return "安静";
        case DisturbanceLevel::Normal:
            return "正常";
        case DisturbanceLevel::Clingy:
            return "粘人";
    }
    return "正常";
}

std::string BehaviorPolicy::personaStyleLabel() const {
    if (personaProfile_.find("公主") != std::string::npos) {
        return "公主系 / 克制关怀";
    }
    if (!personaProfile_.empty()) {
        return personaProfile_ + " / 默认风格";
    }
    return "默认风格";
}

std::string BehaviorPolicy::strategySummary(const brain::BrainState& state) const {
    const DisturbanceLevel level = disturbanceLevel(state);
    std::string summary = "主动策略：";
    summary += disturbanceModeLabel();
    summary += "，冷却 ";
    summary += std::to_string(proactiveCooldownSeconds(level));
    summary += " 秒";
    if (isQuietHour()) {
        summary += "，当前处于安静时段";
    }
    return summary;
}

std::string BehaviorPolicy::statusBarSummary(const brain::BrainState& state) const {
    std::string summary = disturbanceModeLabel();
    summary += " / ";
    if (personaProfile_.find("公主") != std::string::npos) {
        summary += "公主系";
    } else {
        summary += "默认";
    }
    if (isQuietHour()) {
        summary += " / 夜间";
    }
    return summary;
}

std::string BehaviorPolicy::behaviorStatsSummary() const {
    const auto& state = strategyStateService_.state();
    return "关怀 " + std::to_string(state.stats.careCount) +
        " / 问候 " + std::to_string(state.stats.checkInCount) +
        " / 逗弄 " + std::to_string(state.stats.teaseCount) +
        " / 提醒 " + std::to_string(state.stats.reminderCount);
}

std::string BehaviorPolicy::lastProactiveBehaviorLabel() const {
    return strategyStateService_.state().lastProactiveBehavior;
}

std::string BehaviorPolicy::lastUserInteractionLabel() const {
    const auto& state = strategyStateService_.state();
    if (state.lastUserInteractionAt.time_since_epoch().count() == 0) {
        return "从未";
    }
    const auto now = std::chrono::steady_clock::now();
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now - state.lastUserInteractionAt).count();
    if (seconds < 60) {
        return std::to_string(seconds) + " 秒前";
    }
    return std::to_string(seconds / 60) + " 分钟前";
}

bool BehaviorPolicy::isQuietHourActive() const {
    return isQuietHour();
}

void BehaviorPolicy::markProactiveBehavior(ProactiveBehaviorType type) {
    auto& state = strategyStateService_.state();
    switch (type) {
        case ProactiveBehaviorType::Care:
            state.lastProactiveBehavior = "主动关怀";
            ++state.stats.careCount;
            break;
        case ProactiveBehaviorType::CheckIn:
            state.lastProactiveBehavior = "主动问候";
            ++state.stats.checkInCount;
            break;
        case ProactiveBehaviorType::Tease:
            state.lastProactiveBehavior = "主动逗弄";
            ++state.stats.teaseCount;
            break;
        case ProactiveBehaviorType::Reminder:
            state.lastProactiveBehavior = "主动提醒";
            ++state.stats.reminderCount;
            break;
    }
}

void BehaviorPolicy::resetStrategyState() {
    strategyStateService_.reset();
}

void BehaviorPolicy::setBehaviorEnabled(ProactiveBehaviorType type, bool enabled) {
    switch (type) {
        case ProactiveBehaviorType::Care:
            strategyStateService_.state().allowCare = enabled;
            break;
        case ProactiveBehaviorType::CheckIn:
            strategyStateService_.state().allowCheckIn = enabled;
            break;
        case ProactiveBehaviorType::Tease:
            strategyStateService_.state().allowTease = enabled;
            break;
        case ProactiveBehaviorType::Reminder:
            strategyStateService_.state().allowReminder = enabled;
            break;
    }
}

bool BehaviorPolicy::isBehaviorEnabled(ProactiveBehaviorType type) const {
    switch (type) {
        case ProactiveBehaviorType::Care:
            return strategyStateService_.state().allowCare;
        case ProactiveBehaviorType::CheckIn:
            return strategyStateService_.state().allowCheckIn;
        case ProactiveBehaviorType::Tease:
            return strategyStateService_.state().allowTease;
        case ProactiveBehaviorType::Reminder:
            return strategyStateService_.state().allowReminder;
    }
    return true;
}

void BehaviorPolicy::setSilentAtNight(bool enabled) {
    strategyStateService_.state().silentAtNight = enabled;
}

bool BehaviorPolicy::silentAtNight() const {
    return strategyStateService_.state().silentAtNight;
}

MemoryDB::AppStateSnapshot BehaviorPolicy::exportState() const {
    MemoryDB::AppStateSnapshot state;
    state.disturbanceMode = configuredLevel_ == DisturbanceLevel::Quiet ? "quiet" : configuredLevel_ == DisturbanceLevel::Clingy ? "clingy" : "normal";
    const auto& strategy = strategyStateService_.state();
    state.lastNotification = strategy.lastNotification;
    state.lastNotificationAt = strategy.lastNotificationAt.time_since_epoch().count() == 0 ? 0 : now_epoch_seconds();
    state.lastProactiveAt = strategy.lastProactiveAt.time_since_epoch().count() == 0 ? 0 : now_epoch_seconds();
    state.lastUserInteractionAt = strategy.lastUserInteractionAt.time_since_epoch().count() == 0 ? 0 : now_epoch_seconds();
    state.lastProactiveBehavior = strategy.lastProactiveBehavior;
    state.careCount = strategy.stats.careCount;
    state.checkInCount = strategy.stats.checkInCount;
    state.teaseCount = strategy.stats.teaseCount;
    state.reminderCount = strategy.stats.reminderCount;
    state.allowCare = strategy.allowCare;
    state.allowCheckIn = strategy.allowCheckIn;
    state.allowTease = strategy.allowTease;
    state.allowReminder = strategy.allowReminder;
    state.silentAtNight = strategy.silentAtNight;
    return state;
}

void BehaviorPolicy::importState(const MemoryDB::AppStateSnapshot& state) {
    setDisturbanceMode(state.disturbanceMode);
    auto& strategy = strategyStateService_.state();
    strategy.lastNotification = state.lastNotification;
    strategy.lastNotificationAt = restore_steady_from_epoch(state.lastNotificationAt);
    strategy.lastProactiveAt = restore_steady_from_epoch(state.lastProactiveAt);
    strategy.lastUserInteractionAt = restore_steady_from_epoch(state.lastUserInteractionAt);
    strategy.stats.careCount = state.careCount;
    strategy.stats.checkInCount = state.checkInCount;
    strategy.stats.teaseCount = state.teaseCount;
    strategy.stats.reminderCount = state.reminderCount;
    strategy.allowCare = state.allowCare;
    strategy.allowCheckIn = state.allowCheckIn;
    strategy.allowTease = state.allowTease;
    strategy.allowReminder = state.allowReminder;
    strategy.silentAtNight = state.silentAtNight;
    strategy.lastProactiveBehavior = state.lastProactiveBehavior;
}

bool BehaviorPolicy::isQuietHour() const {
    const std::time_t now = std::time(nullptr);
    const std::tm* local = std::localtime(&now);
    if (!local) {
        return false;
    }
    return local->tm_hour < 8 || local->tm_hour >= 23;
}

bool BehaviorPolicy::hasRecentInteractionBurst() const {
    return strategyStateService_.state().recentUserInteractions.size() >= 4;
}

bool BehaviorPolicy::isIdleLongEnough(DisturbanceLevel level) const {
    const auto& state = strategyStateService_.state();
    if (state.lastUserInteractionAt.time_since_epoch().count() == 0) {
        return true;
    }

    const auto now = std::chrono::steady_clock::now();
    const auto idleSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - state.lastUserInteractionAt).count();
    switch (level) {
        case DisturbanceLevel::Quiet:
            return idleSeconds >= 90;
        case DisturbanceLevel::Normal:
            return idleSeconds >= 45;
        case DisturbanceLevel::Clingy:
            return idleSeconds >= 20;
    }
    return idleSeconds >= 45;
}

}  // namespace action
