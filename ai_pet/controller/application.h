#pragma once

#include "controller/module_registry.h"
#include "controller/session.h"
#include <future>
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
    void waitForPendingAI();
    void runChatMode();
    void runCameraMode();
    void runFullMode();
    void runUIMode();

    ModuleRegistry modules_;
    Session session_;
    std::string personaName_;
    int faceDetectInterval_;
    std::future<void> pendingAI_;
};
