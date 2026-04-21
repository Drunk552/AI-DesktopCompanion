#pragma once

#include "brain/brain_types.h"
#include <string>

#include "app/settings_events.h"

class AppEventBus;
class ConfigManager;
class MemoryDB;

namespace action {
class ActionCoordinator;
}

namespace brain {
class BrainController;
class Session;
} 

class StrategySettingsService {
public:
    StrategySettingsService(
        AppEventBus& eventBus,
        ConfigManager& config,
        MemoryDB& memory,
        action::ActionCoordinator& actionCoordinator,
        brain::BrainController& brainController,
        brain::Session& session
    );

    void initialize();
    void emitCurrentState();

private:
    static constexpr const char* kStrategyFilePath = "data/strategy_settings_export.json";
    static constexpr const char* kImportScopeFull = "full";
    static constexpr const char* kImportScopeToggles = "toggles";
    static constexpr const char* kImportScopeStats = "stats";
    void bindEvents();
    void persist();
    void emitState();
    app::settings::events::SettingsStateSnapshot buildSettingsSnapshot(const brain::BrainState& state) const;
    void exportStrategy();
    void importStrategy(const std::string& scope);
    void restoreDefaults();

    AppEventBus& eventBus_;
    ConfigManager& config_;
    MemoryDB& memory_;
    action::ActionCoordinator& actionCoordinator_;
    brain::BrainController& brainController_;
    brain::Session& session_;
    bool bound_ = false;
};
