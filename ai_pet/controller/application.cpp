#include "controller/application.h"
#include "config/config.h"
#include "logger/logger.h"
#include "vision/camera.h"
#include "vision/vision_pipeline.h"
#include "ui/ui.h"
#include <atomic>
#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <poll.h>
#include <thread>
#include <unistd.h>

Application::Application(const std::string& configPath)
    : modules_(configPath)
    , session_(modules_.ai(), modules_.memory(), modules_.personaLoader())
    , faceDetectInterval_(modules_.config().get().face_detect_interval) {}

Application::~Application() {
    session_.cancelPending();
    waitForPendingAI();
}

void Application::setPersonaName(const std::string& name) {
    personaName_ = name;
}

int Application::run(AppMode mode) {
    switch (mode) {
        case AppMode::UI:
            runUIMode();
            return 0;
        case AppMode::Chat:
            runChatMode();
            return 0;
        case AppMode::Camera:
            runCameraMode();
            return 0;
        case AppMode::Full:
            runFullMode();
            return 0;
    }

    return 1;
}

bool Application::initVision() {
    return modules_.visionPipeline().init();
}

void Application::initializeSession() {
    session_.initialize(modules_.config().get().personas_dir, personaName_);
}

void Application::waitForPendingAI() {
    if (pendingAI_.valid()) {
        pendingAI_.wait();
    }
}

void Application::runChatMode() {
    std::cout << "=============================" << std::endl;
    std::cout << "  AI 桌面宠物 - 对话模式" << std::endl;
    std::cout << "  输入 'quit' 或 'exit' 退出" << std::endl;
    std::cout << "=============================" << std::endl;
    std::cout << std::endl;

    initializeSession();

    std::string input;
    while (true) {
        std::cout << "[" << session_.currentAffinityLevel() << " " << session_.currentAffinity() << "] 你: ";
        std::getline(std::cin, input);

        if (input == "quit" || input == "exit") {
            std::cout << "……再见。" << std::endl;
            break;
        }
        if (input.empty()) {
            continue;
        }

        std::cout << "（思考中...）" << std::endl;
        const SessionTurnResult result = session_.handleUserInput(input);
        if (result.success) {
            std::cout << "TA: " << result.reply << std::endl;
        } else {
            std::cout << "TA: ……（网络有点问题，再试一次？）" << std::endl;
        }

        std::cout << std::endl;
    }
}

void Application::runCameraMode() {
    LOGI("Application", "启动摄像头测试模式");

    if (!initVision()) {
        LOGE("Application", "视觉模块初始化失败");
        return;
    }

    if (!modules_.camera().open()) {
        LOGE("Application", "无法打开摄像头，请检查 /tmp/yuyv.sdp 是否存在");
        return;
    }

    LOGI("Application", "摄像头已打开，按 'q' 退出");

    cv::Mat frame;
    while (true) {
        if (modules_.camera().getFrame(frame)) {
            VisionResult vr = modules_.visionPipeline().process(frame);
            if (vr.faceDetected && !vr.emotion.empty()) {
                session_.onEmotionDetected(vr.emotion);
            }
            cv::imshow("Camera - Face & Emotion", vr.annotatedFrame);
        }
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    cv::destroyAllWindows();
    modules_.camera().close();
}

void Application::runFullMode() {
    std::cout << "=============================" << std::endl;
    std::cout << "  AI 桌面宠物 - 完整模式" << std::endl;
    std::cout << "  摄像头 + 表情识别 + 对话" << std::endl;
    std::cout << "  输入 'quit' 或 'exit' 退出" << std::endl;
    std::cout << "  摄像头窗口按 'q' 也可退出" << std::endl;
    std::cout << "=============================" << std::endl;
    std::cout << std::endl;

    const bool visionReady = initVision();
    bool cameraReady = false;
    if (visionReady) {
        cameraReady = modules_.camera().open();
    }

    if (!cameraReady) {
        LOGW("Application", "摄像头未就绪，将在纯对话模式下运行");
    }

    initializeSession();

    std::atomic<bool> running{true};

    std::thread chatThread([this, &running]() {
        std::string input;
        bool promptShown = false;
        while (running) {
            if (!promptShown) {
                std::cout << "[当前情绪: " << session_.currentEmotion() << "] 你: " << std::flush;
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
            const SessionTurnResult result = session_.handleUserInput(input);
            if (result.success) {
                std::cout << "TA: " << result.reply << std::endl;
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
            if (modules_.camera().getFrame(frame)) {
                LOGI("Application", "收到第一帧，摄像头正常");
                break;
            }
            emptyCount++;
            cv::waitKey(10);
        }
        if (emptyCount >= 300) {
            LOGW("Application", "摄像头超时无数据，继续尝试...");
        }

        int frameCount = 0;
        while (running) {
            if (modules_.camera().getFrame(frame)) {
                if (frameCount % faceDetectInterval_ == 0) {
                    VisionResult vr = modules_.visionPipeline().process(frame);
                    if (vr.faceDetected && !vr.emotion.empty()) {
                        session_.onEmotionDetected(vr.emotion);
                    }
                    cv::imshow("AI Pet - Camera", vr.annotatedFrame);
                }
                frameCount++;
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
    session_.cancelPending();
    if (chatThread.joinable()) {
        chatThread.join();
    }
    modules_.camera().close();
}

void Application::runUIMode() {
    LOGI("Application", "启动 UI 模式");

    const auto& cfg = modules_.config().get();
    if (!modules_.ui().init(cfg.ui_window_width, cfg.ui_window_height)) {
        LOGE("Application", "UI 初始化失败");
        return;
    }

    const bool visionReady = initVision();
    bool cameraReady = false;
    if (visionReady) {
        cameraReady = modules_.camera().open();
    }
    if (!cameraReady) {
        LOGW("Application", "摄像头未就绪，UI 将在无摄像头模式下运行");
    }

    initializeSession();

    std::atomic<bool> running{true};

    modules_.ui().setSendCallback([this, &running](const std::string& input) {
        if (input == "quit" || input == "exit") {
            running = false;
            session_.cancelPending();
            return;
        }

        if (input.empty()) {
            return;
        }

        if (!session_.tryBeginTurn()) {
            LOGW("Application", "AI 正在处理中，请稍候");
            return;
        }

        modules_.ui().addChatMessage(input, true);
        modules_.ui().setThinking(true);

        waitForPendingAI();
        pendingAI_ = std::async(std::launch::async, [this, input]() {
            try {
                const SessionTurnResult result = session_.handleReservedUserInput(input, 180000);
                modules_.ui().setThinking(false);

                if (result.success) {
                    modules_.ui().addChatMessage(result.reply, false);
                    modules_.ui().updateEmotion(result.emotion);
                    return;
                }

                if (!result.accepted) {
                    return;
                }

                modules_.ui().addChatMessage("……抱歉，网络好像有点问题", false);
            } catch (const std::exception& e) {
                LOGE("Application", "处理消息异常: " + std::string(e.what()));
                modules_.ui().setThinking(false);
                modules_.ui().addChatMessage("系统错误: " + std::string(e.what()), false);
            } catch (...) {
                LOGE("Application", "处理消息发生未知异常");
                modules_.ui().setThinking(false);
                modules_.ui().addChatMessage("系统错误", false);
            }
        });
    });

    std::thread visionThread;
    if (cameraReady) {
        visionThread = std::thread([this, &running]() {
            cv::Mat frame;
            int frameCount = 0;
            while (running) {
                if (modules_.camera().getFrame(frame)) {
                    if (frameCount % faceDetectInterval_ == 0) {
                        VisionResult vr = modules_.visionPipeline().process(frame, false);
                        if (vr.faceDetected && !vr.emotion.empty()) {
                            session_.onEmotionDetected(vr.emotion);
                            modules_.ui().updateEmotion(vr.emotion);
                        }
                    }
                    modules_.ui().updateCameraFrame(frame);
                    frameCount++;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        });
        LOGI("Application", "摄像头后台线程已启动");
    }

    while (running) {
        if (!modules_.ui().tick()) {
            running = false;
            break;
        }
    }

    running = false;
    session_.cancelPending();
    if (visionThread.joinable()) {
        visionThread.join();
    }
    waitForPendingAI();
    modules_.camera().close();
    modules_.ui().shutdown();
    LOGI("Application", "UI 模式已退出");
}
