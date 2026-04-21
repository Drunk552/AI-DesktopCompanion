#pragma once

#include <functional>

class AppEventBus;
class ModuleRegistry;
class StrategySettingsService;

namespace brain {
class Session;
class BrainController;
}

struct RuntimeContext {
    AppEventBus& eventBus;
    ModuleRegistry& modules;
    brain::Session& session;
    brain::BrainController& brainController;
    StrategySettingsService& strategySettingsService;
    int faceDetectInterval;
    std::function<bool()> initVision;
    std::function<void()> initializeSession;
};
