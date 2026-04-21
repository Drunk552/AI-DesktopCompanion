#pragma once

#include "action/action_coordinator.h"
#include "app/app_event_bus.h"
#include "app/module_registry.h"
#include "app/strategy_settings_service.h"
#include "brain/brain_controller.h"
#include "brain/session.h"
#include <string>

enum class AppMode {
    UI,
    Chat,
    Camera,
    Full,
};

class Application {
public:
    explicit Application(const std::string& configPath = "config.json");
    ~Application();

    void setPersonaName(const std::string& name);
    int run(AppMode mode);

private:
    bool initVision();
    void initializeSession();

    AppEventBus appEventBus_;
    action::ActionCoordinator actionCoordinator_;
    ModuleRegistry modules_;
    brain::Session session_;
    brain::BrainController brainController_;
    StrategySettingsService strategySettingsService_;
    std::string personaName_;
    int faceDetectInterval_;
};
