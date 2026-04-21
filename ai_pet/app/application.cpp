#include "app/application.h"
#include "app/mode_runner_camera.h"
#include "app/mode_runner_chat.h"
#include "app/runtime_context.h"
#include "app/mode_runner_full.h"
#include "app/mode_runner_ui.h"
#include "brain/conversation_orchestrator.h"
#include "perception/vision_pipeline.h"
#include "shared/config/config.h"

Application::Application(const std::string& configPath)
    : actionCoordinator_(appEventBus_)
    , modules_(configPath)
    , session_(modules_.brainServices().ai, modules_.brainServices().memory, modules_.brainServices().personaLoader)
    , brainController_(session_.orchestrator(), session_.emotionState(), session_.relationState(), actionCoordinator_)
    , strategySettingsService_(appEventBus_, modules_.runtimeServices().config, modules_.brainServices().memory, actionCoordinator_, brainController_, session_)
    , faceDetectInterval_(modules_.config().get().face_detect_interval) {}

Application::~Application() {
    brainController_.shutdown();
    session_.orchestrator().cancelPending();
}

void Application::setPersonaName(const std::string& name) {
    personaName_ = name;
}

int Application::run(AppMode mode) {
    RuntimeContext context{
        appEventBus_,
        modules_,
        session_,
        brainController_,
        strategySettingsService_,
        faceDetectInterval_,
        [this]() { return initVision(); },
        [this]() { initializeSession(); },
    };

    switch (mode) {
        case AppMode::UI:
            UIModeRunner(context).run();
            return 0;
        case AppMode::Chat:
            ChatModeRunner(context).run();
            return 0;
        case AppMode::Camera:
            CameraModeRunner(context).run();
            return 0;
        case AppMode::Full:
            FullModeRunner(context).run();
            return 0;
    }

    return 1;
}

bool Application::initVision() {
    auto runtime = modules_.runtimeServices();
    return runtime.visionPipeline.init();
}

void Application::initializeSession() {
    auto runtime = modules_.runtimeServices();
    session_.initialize(runtime.config.get().personas_dir, personaName_);
    strategySettingsService_.initialize();
}
