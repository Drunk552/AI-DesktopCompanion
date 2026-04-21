#pragma once

#include <string>

namespace app::settings::events {

struct SettingsStateSnapshot {
    std::string strategyFilePath;
    std::string personaName;
    std::string disturbanceMode;
    std::string personaStyle;
    std::string strategySummary;
    std::string statusBrief;
    std::string lastBehavior;
    std::string lastInteraction;
    std::string quietHour;
    std::string behaviorStats;
    bool checkInEnabled = true;
    bool teaseEnabled = true;
    bool reminderEnabled = true;
    bool silentNightEnabled = false;
    std::string importResult = "未执行";
    std::string importError = "无";
};

struct DisturbanceModeSetCommand {
    std::string mode;
};

struct ToggleBehaviorCommand {
    std::string key;
};

struct StrategyImportCommand {
    std::string scope;
};

struct SimpleActionCommand {};

inline constexpr const char* kDisturbanceModeSet = "app.settings.disturbance_mode.set";
inline constexpr const char* kBehaviorCheckInToggle = "app.settings.behavior.checkin.toggle";
inline constexpr const char* kBehaviorTeaseToggle = "app.settings.behavior.tease.toggle";
inline constexpr const char* kBehaviorReminderToggle = "app.settings.behavior.reminder.toggle";
inline constexpr const char* kBehaviorSilentNightToggle = "app.settings.behavior.silent_night.toggle";
inline constexpr const char* kStrategyReset = "app.settings.strategy.reset";
inline constexpr const char* kStrategyExport = "app.settings.strategy.export";
inline constexpr const char* kStrategyImport = "app.settings.strategy.import";
inline constexpr const char* kStrategyRestoreDefaults = "app.settings.strategy.restore_defaults";
inline constexpr const char* kStateSnapshotChanged = "app.settings.state.snapshot.changed";

inline constexpr const char* kStrategyPathCurrent = "app.settings.strategy.path.current";
inline constexpr const char* kPersonaNameCurrent = "app.settings.persona_name.current";
inline constexpr const char* kDisturbanceModeCurrent = "app.settings.disturbance_mode.current";
inline constexpr const char* kPersonaStyleCurrent = "app.settings.persona_style.current";
inline constexpr const char* kStrategySummaryCurrent = "app.settings.strategy_summary.current";
inline constexpr const char* kStatusBriefCurrent = "app.settings.status_brief.current";
inline constexpr const char* kLastBehaviorCurrent = "app.settings.last_behavior.current";
inline constexpr const char* kLastInteractionCurrent = "app.settings.last_interaction.current";
inline constexpr const char* kQuietHourCurrent = "app.settings.quiet_hour.current";
inline constexpr const char* kBehaviorStatsCurrent = "app.settings.behavior_stats.current";
inline constexpr const char* kBehaviorToggleCheckIn = "app.settings.behavior.toggle.checkin";
inline constexpr const char* kBehaviorToggleTease = "app.settings.behavior.toggle.tease";
inline constexpr const char* kBehaviorToggleReminder = "app.settings.behavior.toggle.reminder";
inline constexpr const char* kBehaviorToggleSilentNight = "app.settings.behavior.toggle.silent_night";
inline constexpr const char* kStrategyImportResult = "app.settings.strategy.import.result";
inline constexpr const char* kStrategyImportError = "app.settings.strategy.import.error";

}  // namespace app::settings::events
