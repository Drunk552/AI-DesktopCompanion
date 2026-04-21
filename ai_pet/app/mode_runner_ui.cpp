#include "app/mode_runner_ui.h"
#include "app/idle_monitor.h"
#include "app/runtime_context.h"
#include "app/module_registry.h"
#include "app/strategy_settings_service.h"
#include "brain/brain_controller.h"
#include "brain/conversation_orchestrator.h"
#include "brain/brain_events.h"
#include "brain/session.h"
#include "perception/camera.h"
#include "perception/vision_pipeline.h"
#include "shared/config/config.h"
#include "shared/logger/logger.h"
#include "ui/ui.h"
#include <atomic>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <thread>

UIModeRunner::UIModeRunner(RuntimeContext& context)
    : context_(context) {}

void UIModeRunner::run() {
    LOGI("Application", "启动 UI 模式");
    auto runtime = context_.modules.runtimeServices();
    context_.brainController.setActiveMode("ui");

    const auto& cfg = runtime.config.get();
    runtime.ui.setEventBus(&context_.eventBus);
    if (!runtime.ui.init(cfg.ui_window_width, cfg.ui_window_height)) {
        LOGE("Application", "UI 初始化失败");
        return;
    }

    const bool visionReady = context_.initVision();
    bool cameraReady = false;
    if (visionReady) {
        cameraReady = runtime.camera.open();
    }
    if (!cameraReady) {
        LOGW("Application", "摄像头未就绪，UI 将在无摄像头模式下运行");
    }

    context_.initializeSession();
    context_.brainController.setPersonaName(context_.session.personaName());

    std::atomic<bool> running{true};
    IdleMonitor idleMonitor;
    context_.brainController.init(context_.eventBus, [&running]() { running = false; });
    context_.brainController.syncUIState();
    context_.strategySettingsService.emitCurrentState();
    idleMonitor.start(context_.eventBus, running);

    std::thread visionThread;
    if (cameraReady) {
        visionThread = std::thread([this, &running, runtime]() mutable {
            cv::Mat frame;
            int frameCount = 0;
            while (running) {
                if (runtime.camera.getFrame(frame)) {
                    if (frameCount % context_.faceDetectInterval == 0) {
                        VisionResult vr = runtime.visionPipeline.process(frame, false);
                        if (vr.faceDetected && !vr.emotion.empty()) {
                            context_.eventBus.emitTyped(brain::events::kPerceptionEmotionDetected, brain::events::PerceptionEmotionEvent{vr.emotion});
                        }
                    }
                    runtime.ui.updateCameraFrame(frame);
                    ++frameCount;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        });
        LOGI("Application", "摄像头后台线程已启动");
    }

    while (running) {
        if (!runtime.ui.tick()) {
            running = false;
            break;
        }
    }

    running = false;
    context_.session.orchestrator().cancelPending();
    idleMonitor.stop();
    if (visionThread.joinable()) {
        visionThread.join();
    }
    context_.brainController.shutdown();
    runtime.camera.close();
    runtime.ui.shutdown();
    LOGI("Application", "UI 模式已退出");
}
