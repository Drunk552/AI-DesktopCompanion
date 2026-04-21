#include "app/strategy_settings_service.h"
#include "action/action_coordinator.h"
#include "action/action_events.h"
#include "action/behavior_policy.h"
#include "app/settings_events.h"
#include "app/app_event_bus.h"
#include "brain/brain_controller.h"
#include "brain/brain_events.h"
#include "brain/brain_types.h"
#include "brain/session.h"
#include "intelligence/memory/memory_db.h"
#include "shared/config/config.h"
#include "shared/logger/logger.h"
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

StrategySettingsService::StrategySettingsService(
    AppEventBus& eventBus,
    ConfigManager& config,
    MemoryDB& memory,
    action::ActionCoordinator& actionCoordinator,
    brain::BrainController& brainController,
    brain::Session& session
)
    : eventBus_(eventBus)
    , config_(config)
    , memory_(memory)
    , actionCoordinator_(actionCoordinator)
    , brainController_(brainController)
    , session_(session) {}

void StrategySettingsService::initialize() {
    actionCoordinator_.setDisturbanceMode(config_.get().disturbance_mode);
    actionCoordinator_.behaviorPolicy().importState(memory_.loadAppState());
    actionCoordinator_.setPersonaProfile(session_.personaName());
    bindEvents();
}

void StrategySettingsService::emitCurrentState() {
    emitState();
}

void StrategySettingsService::bindEvents() {
    if (bound_) {
        return;
    }
    bound_ = true;

    eventBus_.subscribeTyped<brain::BrainStateChangedEvent>(brain::events::kBrainStateChanged, [this](const brain::BrainStateChangedEvent&) {
        emitState();
    });

    eventBus_.subscribeTyped<app::settings::events::DisturbanceModeSetCommand>(app::settings::events::kDisturbanceModeSet, [this](const app::settings::events::DisturbanceModeSetCommand& command) {
        config_.get().disturbance_mode = command.mode;
        actionCoordinator_.setDisturbanceMode(command.mode);
        persist();
        emitState();
    });
    eventBus_.subscribeTyped<app::settings::events::ToggleBehaviorCommand>(app::settings::events::kBehaviorCheckInToggle, [this](const app::settings::events::ToggleBehaviorCommand&) {
        bool enabled = !actionCoordinator_.behaviorPolicy().isBehaviorEnabled(action::ProactiveBehaviorType::CheckIn);
        actionCoordinator_.behaviorPolicy().setBehaviorEnabled(action::ProactiveBehaviorType::CheckIn, enabled);
        persist();
        emitState();
    });
    eventBus_.subscribeTyped<app::settings::events::ToggleBehaviorCommand>(app::settings::events::kBehaviorTeaseToggle, [this](const app::settings::events::ToggleBehaviorCommand&) {
        bool enabled = !actionCoordinator_.behaviorPolicy().isBehaviorEnabled(action::ProactiveBehaviorType::Tease);
        actionCoordinator_.behaviorPolicy().setBehaviorEnabled(action::ProactiveBehaviorType::Tease, enabled);
        persist();
        emitState();
    });
    eventBus_.subscribeTyped<app::settings::events::ToggleBehaviorCommand>(app::settings::events::kBehaviorReminderToggle, [this](const app::settings::events::ToggleBehaviorCommand&) {
        bool enabled = !actionCoordinator_.behaviorPolicy().isBehaviorEnabled(action::ProactiveBehaviorType::Reminder);
        actionCoordinator_.behaviorPolicy().setBehaviorEnabled(action::ProactiveBehaviorType::Reminder, enabled);
        persist();
        emitState();
    });
    eventBus_.subscribeTyped<app::settings::events::ToggleBehaviorCommand>(app::settings::events::kBehaviorSilentNightToggle, [this](const app::settings::events::ToggleBehaviorCommand&) {
        bool enabled = !actionCoordinator_.behaviorPolicy().silentAtNight();
        actionCoordinator_.behaviorPolicy().setSilentAtNight(enabled);
        persist();
        emitState();
    });
    eventBus_.subscribeTyped<app::settings::events::SimpleActionCommand>(app::settings::events::kStrategyReset, [this](const app::settings::events::SimpleActionCommand&) {
        actionCoordinator_.behaviorPolicy().resetStrategyState();
        persist();
        emitState();
    });
    eventBus_.subscribeTyped<app::settings::events::SimpleActionCommand>(app::settings::events::kStrategyExport, [this](const app::settings::events::SimpleActionCommand&) {
        exportStrategy();
        eventBus_.emitTyped(action::events::kNotificationShow, action::events::NotificationEvent{"策略已导出到 data/strategy_settings_export.json"});
    });
    eventBus_.subscribeTyped<app::settings::events::StrategyImportCommand>(app::settings::events::kStrategyImport, [this](const app::settings::events::StrategyImportCommand& command) {
        importStrategy(command.scope.empty() ? kImportScopeFull : command.scope);
        persist();
        emitState();
        eventBus_.emitTyped(action::events::kNotificationShow, action::events::NotificationEvent{"已导入策略文件"});
    });
    eventBus_.subscribeTyped<app::settings::events::SimpleActionCommand>(app::settings::events::kStrategyRestoreDefaults, [this](const app::settings::events::SimpleActionCommand&) {
        restoreDefaults();
        persist();
        emitState();
        eventBus_.emitTyped(action::events::kNotificationShow, action::events::NotificationEvent{"已恢复默认策略"});
    });
}

void StrategySettingsService::persist() {
    config_.save();
    memory_.saveAppState(actionCoordinator_.behaviorPolicy().exportState());
}

void StrategySettingsService::emitState() {
    const brain::BrainState state = brainController_.currentState();
    const auto snapshot = buildSettingsSnapshot(state);
    eventBus_.emitTyped(app::settings::events::kStateSnapshotChanged, snapshot);
}

app::settings::events::SettingsStateSnapshot StrategySettingsService::buildSettingsSnapshot(const brain::BrainState& state) const {
    app::settings::events::SettingsStateSnapshot snapshot;
    snapshot.strategyFilePath = kStrategyFilePath;
    snapshot.personaName = state.personaName.empty() ? std::string("紫金公主") : state.personaName;
    snapshot.disturbanceMode = config_.get().disturbance_mode;
    snapshot.personaStyle = state.personaStyle;
    snapshot.strategySummary = actionCoordinator_.behaviorPolicy().strategySummary(state);
    snapshot.statusBrief = actionCoordinator_.behaviorPolicy().statusBarSummary(state);
    snapshot.lastBehavior = state.lastProactiveBehavior;
    snapshot.lastInteraction = state.lastUserInteractionAt;
    snapshot.quietHour = state.quietHourActive ? "是" : "否";
    snapshot.behaviorStats = actionCoordinator_.behaviorPolicy().behaviorStatsSummary();
    snapshot.checkInEnabled = actionCoordinator_.behaviorPolicy().isBehaviorEnabled(action::ProactiveBehaviorType::CheckIn);
    snapshot.teaseEnabled = actionCoordinator_.behaviorPolicy().isBehaviorEnabled(action::ProactiveBehaviorType::Tease);
    snapshot.reminderEnabled = actionCoordinator_.behaviorPolicy().isBehaviorEnabled(action::ProactiveBehaviorType::Reminder);
    snapshot.silentNightEnabled = actionCoordinator_.behaviorPolicy().silentAtNight();
    return snapshot;
}

void StrategySettingsService::exportStrategy() {
    const auto snapshot = actionCoordinator_.behaviorPolicy().exportState();
    json j = {
        {"disturbance_mode", snapshot.disturbanceMode},
        {"last_proactive_behavior", snapshot.lastProactiveBehavior},
        {"care_count", snapshot.careCount},
        {"checkin_count", snapshot.checkInCount},
        {"tease_count", snapshot.teaseCount},
        {"reminder_count", snapshot.reminderCount},
        {"allow_care", snapshot.allowCare},
        {"allow_checkin", snapshot.allowCheckIn},
        {"allow_tease", snapshot.allowTease},
        {"allow_reminder", snapshot.allowReminder},
        {"silent_at_night", snapshot.silentAtNight}
    };
    std::ofstream out("data/strategy_settings_export.json");
    out << j.dump(2);
}

void StrategySettingsService::restoreDefaults() {
    config_.get().disturbance_mode = "normal";
    actionCoordinator_.setDisturbanceMode("normal");
    actionCoordinator_.behaviorPolicy().resetStrategyState();
    actionCoordinator_.behaviorPolicy().setBehaviorEnabled(action::ProactiveBehaviorType::Care, true);
    actionCoordinator_.behaviorPolicy().setBehaviorEnabled(action::ProactiveBehaviorType::CheckIn, true);
    actionCoordinator_.behaviorPolicy().setBehaviorEnabled(action::ProactiveBehaviorType::Tease, true);
    actionCoordinator_.behaviorPolicy().setBehaviorEnabled(action::ProactiveBehaviorType::Reminder, true);
    actionCoordinator_.behaviorPolicy().setSilentAtNight(false);
}

void StrategySettingsService::importStrategy(const std::string& scope) {
    std::ifstream in(kStrategyFilePath);
    if (!in.is_open()) {
        auto snapshot = buildSettingsSnapshot(brainController_.currentState());
        snapshot.importResult = "失败";
        snapshot.importError = std::string("未找到文件: ") + kStrategyFilePath;
        eventBus_.emitTyped(app::settings::events::kStateSnapshotChanged, snapshot);
        eventBus_.emitTyped(action::events::kNotificationShow, action::events::NotificationEvent{std::string("未找到导入文件 ") + kStrategyFilePath});
        return;
    }

    json j;
    try {
        in >> j;
    } catch (const std::exception& e) {
        auto snapshot = buildSettingsSnapshot(brainController_.currentState());
        snapshot.importResult = "失败";
        snapshot.importError = std::string("解析失败: ") + e.what();
        eventBus_.emitTyped(app::settings::events::kStateSnapshotChanged, snapshot);
        eventBus_.emitTyped(action::events::kNotificationShow, action::events::NotificationEvent{"策略文件解析失败"});
        return;
    }
    auto strategySnapshot = actionCoordinator_.behaviorPolicy().exportState();
    if (scope == kImportScopeFull || scope == kImportScopeToggles) {
        if (j.contains("disturbance_mode")) strategySnapshot.disturbanceMode = j["disturbance_mode"].get<std::string>();
        if (j.contains("allow_care")) strategySnapshot.allowCare = j["allow_care"].get<bool>();
        if (j.contains("allow_checkin")) strategySnapshot.allowCheckIn = j["allow_checkin"].get<bool>();
        if (j.contains("allow_tease")) strategySnapshot.allowTease = j["allow_tease"].get<bool>();
        if (j.contains("allow_reminder")) strategySnapshot.allowReminder = j["allow_reminder"].get<bool>();
        if (j.contains("silent_at_night")) strategySnapshot.silentAtNight = j["silent_at_night"].get<bool>();
        config_.get().disturbance_mode = strategySnapshot.disturbanceMode;
        actionCoordinator_.setDisturbanceMode(strategySnapshot.disturbanceMode);
    }
    if (scope == kImportScopeFull || scope == kImportScopeStats) {
        if (j.contains("last_proactive_behavior")) strategySnapshot.lastProactiveBehavior = j["last_proactive_behavior"].get<std::string>();
        if (j.contains("care_count")) strategySnapshot.careCount = j["care_count"].get<int>();
        if (j.contains("checkin_count")) strategySnapshot.checkInCount = j["checkin_count"].get<int>();
        if (j.contains("tease_count")) strategySnapshot.teaseCount = j["tease_count"].get<int>();
        if (j.contains("reminder_count")) strategySnapshot.reminderCount = j["reminder_count"].get<int>();
    }

    actionCoordinator_.behaviorPolicy().importState(strategySnapshot);
    auto settingsSnapshot = buildSettingsSnapshot(brainController_.currentState());
    if (scope == kImportScopeToggles) {
        settingsSnapshot.importResult = "成功（只导入开关）";
    } else if (scope == kImportScopeStats) {
        settingsSnapshot.importResult = "成功（只导入统计）";
    } else {
        settingsSnapshot.importResult = "成功（全量）";
    }
    settingsSnapshot.importError = "无";
    eventBus_.emitTyped(app::settings::events::kStateSnapshotChanged, settingsSnapshot);
}
