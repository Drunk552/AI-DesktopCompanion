#include "app/mode_runner_full.h"
#include "app/idle_monitor.h"
#include "app/runtime_context.h"
#include "app/module_registry.h"
#include "brain/brain_controller.h"
#include "brain/conversation_orchestrator.h"
#include "brain/emotion_state.h"
#include "brain/brain_events.h"
#include "brain/session.h"
#include "perception/camera.h"
#include "perception/vision_pipeline.h"
#include "shared/logger/logger.h"
#include <atomic>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <poll.h>
#include <thread>
#include <unistd.h>

FullModeRunner::FullModeRunner(RuntimeContext& context)
    : context_(context) {}

void FullModeRunner::run() {
    std::cout << "=============================" << std::endl;
    std::cout << "  AI 桌面宠物 - 完整模式" << std::endl;
    std::cout << "  摄像头 + 表情识别 + 对话" << std::endl;
    std::cout << "  输入 'quit' 或 'exit' 退出" << std::endl;
    std::cout << "  摄像头窗口按 'q' 也可退出" << std::endl;
    std::cout << "=============================" << std::endl;
    std::cout << std::endl;

    context_.brainController.setActiveMode("full");
    auto runtime = context_.modules.runtimeServices();
    const bool visionReady = context_.initVision();
    bool cameraReady = false;
    if (visionReady) {
        cameraReady = runtime.camera.open();
    }
    if (!cameraReady) {
        LOGW("Application", "摄像头未就绪，将在纯对话模式下运行");
    }

    context_.initializeSession();
    context_.brainController.setPersonaName(context_.session.personaName());

    std::atomic<bool> running{true};
    IdleMonitor idleMonitor;
    context_.brainController.init(context_.eventBus, [&running]() { running = false; });
    idleMonitor.start(context_.eventBus, running);

    std::thread chatThread([this, &running]() {
        std::string input;
        bool promptShown = false;
        while (running) {
            if (!promptShown) {
                std::cout << "[当前情绪: " << context_.session.emotionState().currentEmotion() << "] 你: " << std::flush;
                promptShown = true;
            }

            pollfd stdinPoll{STDIN_FILENO, POLLIN, 0};
            const int pollResult = poll(&stdinPoll, 1, 100);
            if (pollResult <= 0) {
                continue;
            }
            if ((stdinPoll.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
                running = false;
                break;
            }
            if ((stdinPoll.revents & POLLIN) == 0) {
                continue;
            }
            if (!std::getline(std::cin, input)) {
                running = false;
                break;
            }

            promptShown = false;
            if (!running) {
                break;
            }
            if (input == "quit" || input == "exit") {
                std::cout << "……再见。" << std::endl;
                running = false;
                break;
            }
            if (input.empty()) {
                continue;
            }

            std::cout << "（思考中...）" << std::endl;
            const brain::SessionTurnResult result = context_.brainController.processUserTextSync(input);
            if (result.success) {
                std::cout << "TA: " << result.reply << std::endl;
            } else if (!result.accepted) {
                std::cout << "TA: ……（我还在想上一句，你先等等）" << std::endl;
            } else {
                std::cout << "TA: ……（网络有点问题）" << std::endl;
            }
            std::cout << std::endl;
        }
    });

    if (cameraReady) {
        LOGI("Application", "摄像头已启动，等待第一帧...");
        cv::Mat frame;
        int emptyCount = 0;
        while (running && emptyCount < 300) {
            if (runtime.camera.getFrame(frame)) {
                LOGI("Application", "收到第一帧，摄像头正常");
                break;
            }
            ++emptyCount;
            cv::waitKey(10);
        }
        if (emptyCount >= 300) {
            LOGW("Application", "摄像头超时无数据，继续尝试...");
        }

        int frameCount = 0;
        while (running) {
            if (runtime.camera.getFrame(frame)) {
                if (frameCount % context_.faceDetectInterval == 0) {
                    VisionResult vr = runtime.visionPipeline.process(frame);
                    if (vr.faceDetected && !vr.emotion.empty()) {
                        context_.eventBus.emitTyped(brain::events::kPerceptionEmotionDetected, brain::events::PerceptionEmotionEvent{vr.emotion});
                    }
                    cv::imshow("AI Pet - Camera", vr.annotatedFrame);
                }
                ++frameCount;
            }
            const int key = cv::waitKey(1);
            if (key == 'q' || key == 'Q') {
                running = false;
                break;
            }
        }
        cv::destroyAllWindows();
    }

    running = false;
    context_.session.orchestrator().cancelPending();
    idleMonitor.stop();
    if (chatThread.joinable()) {
        chatThread.join();
    }
    runtime.camera.close();
}
