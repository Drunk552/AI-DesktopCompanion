#include "controller/brain.h"
#include "ai/personality.h"
#include "logger/logger.h"
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

Brain::Brain(const std::string& configPath)
    : config_(ConfigManager::instance())
    , camera_(config_.get().camera_sdp_path)
    , faceDetector_(config_.get().face_model_path)
    , ai_(config_.get().ollama_url, config_.get().model_name)
    , memory_(config_.get().database_path)
    , currentEmotion_("平静")
    , faceDetectInterval_(config_.get().face_detect_interval)
    , emotionStableThreshold_(config_.get().emotion_stable_threshold)
{
    config_.load(configPath);
    LOGI("Brain", "配置已加载");
}

Brain::~Brain() {
    cancelAI();
}

void Brain::cancelAI() {
    cancelled_.store(true);
    ai_.cancel();
}

bool Brain::waitForAI(std::future<std::string>& future, std::string& result, int timeoutMs) {
    auto status = future.wait_for(std::chrono::milliseconds(timeoutMs));
    if (status == std::future_status::ready) {
        try {
            result = future.get();
            return true;
        } catch (const std::exception& e) {
            LOGE("Brain", "AI 调用异常: " + std::string(e.what()));
            return false;
        }
    } else if (status == std::future_status::timeout) {
        LOGW("Brain", "AI 调用超时");
        cancelled_.store(true);
        ai_.cancel();
        return false;
    }
    return false;
}

void Brain::setPersonaName(const std::string& name) {
    personaName_ = name;
}

bool Brain::loadPersona() {
    const auto& personasDir = config_.get().personas_dir;
    if (!personaName_.empty()) {
        std::string dir = personasDir + "/" + personaName_;
        if (personaLoader_.load(dir)) {
            LOGI("Brain", "已加载角色: " + personaLoader_.getData().name);
            return true;
        } else {
            LOGE("Brain", "指定角色 '" + personaName_ + "' 加载失败");
            return false;
        }
    } else {
        if (personaLoader_.autoLoad(personasDir)) {
            LOGI("Brain", "已加载角色: " + personaLoader_.getData().name);
            return true;
        } else {
            LOGW("Brain", "未找到角色文件，使用默认人格");
            return false;
        }
    }
}

const PersonaData* Brain::getPersonaPtr() const {
    return personaLoader_.isLoaded() ? &personaLoader_.getData() : nullptr;
}

bool Brain::initVision() {
    if (!faceDetector_.load()) {
        LOGE("Brain", "人脸检测模型加载失败");
        return false;
    }
    if (!emotionRecognizer_.load()) {
        LOGE("Brain", "表情识别模型加载失败");
        return false;
    }
    return true;
}

void Brain::runChatMode() {
    std::cout << "=============================" << std::endl;
    std::cout << "  AI 桌面宠物 - 对话模式" << std::endl;
    std::cout << "  输入 'quit' 或 'exit' 退出" << std::endl;
    std::cout << "=============================" << std::endl;
    std::cout << std::endl;

    if (memory_.open()) {
        LOGI("Brain", "记忆系统已启动");
    }

    loadPersona();

    int affinity = memory_.getAffinity();
    AffinityLevel level = getAffinityLevel(affinity);
    LOGI("Brain", "好感度: " + std::to_string(affinity) + "/100 (" + getAffinityLevelName(level) + ")");

    std::string input;
    while (true) {
        std::cout << "[" << getAffinityLevelName(getAffinityLevel(memory_.getAffinity())) << " " << memory_.getAffinity() << "] 你: ";
        std::getline(std::cin, input);

        if (input == "quit" || input == "exit" || input.empty()) {
            if (input == "quit" || input == "exit") {
                std::cout << "……再见。" << std::endl;
                break;
            }
            continue;
        }

        cancelled_.store(false);
        int aff = memory_.getAffinity();
        std::string emotionTrend = memory_.getEmotionTrend();
        std::string context = memory_.getContext(5);

        std::string prompt = buildPrompt(input, currentEmotion_, context, aff, emotionTrend, getPersonaPtr());

        std::cout << "（思考中...）" << std::endl;
        
        auto future = ai_.chatAsync(prompt);
        std::string reply;
        
        if (waitForAI(future, reply)) {
            if (reply.empty()) {
                std::cout << "TA: ……（沉默）" << std::endl;
            } else {
                std::cout << "TA: " << reply << std::endl;
            }
        } else {
            std::cout << "TA: ……（网络有点问题，再试一次？）" << std::endl;
        }

        memory_.saveChat(input, reply.empty() ? "……" : reply, currentEmotion_);
        memory_.recordEmotion(currentEmotion_);
        memory_.incrementChatCount();
        int delta = calculateAffinityDelta(currentEmotion_, input);
        memory_.updateAffinity(delta);

        std::cout << std::endl;
    }
}

void Brain::runCameraMode() {
    LOGI("Brain", "启动摄像头测试模式");

    if (!initVision()) {
        LOGE("Brain", "视觉模块初始化失败");
        return;
    }

    if (!camera_.open()) {
        LOGE("Brain", "无法打开摄像头，请检查 /tmp/yuyv.sdp 是否存在");
        return;
    }

    LOGI("Brain", "摄像头已打开，按 'q' 退出");

    cv::Mat frame;
    while (true) {
        if (camera_.getFrame(frame)) {
            auto faces = faceDetector_.detect(frame);

            for (const auto& face : faces) {
                cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);

                cv::Mat faceROI = frame(face);
                std::string emotion = emotionRecognizer_.recognize(faceROI);

                cv::putText(frame, emotion, cv::Point(face.x, face.y - 10),
                            cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);

                currentEmotion_ = emotion;
            }

            cv::imshow("Camera - Face & Emotion", frame);
        }
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    cv::destroyAllWindows();
    camera_.close();
}

void Brain::runFullMode() {
    std::cout << "=============================" << std::endl;
    std::cout << "  AI 桌面宠物 - 完整模式" << std::endl;
    std::cout << "  摄像头 + 表情识别 + 对话" << std::endl;
    std::cout << "  输入 'quit' 或 'exit' 退出" << std::endl;
    std::cout << "  摄像头窗口按 'q' 也可退出" << std::endl;
    std::cout << "=============================" << std::endl;
    std::cout << std::endl;

    bool visionReady = initVision();
    bool cameraReady = false;
    if (visionReady) {
        cameraReady = camera_.open();
    }

    if (!cameraReady) {
        LOGW("Brain", "摄像头未就绪，将在纯对话模式下运行");
    }

    if (memory_.open()) {
        LOGI("Brain", "记忆系统已启动");
    }

    loadPersona();

    std::atomic<bool> running{true};

    std::thread chatThread([&]() {
        std::string input;
        while (running) {
            {
                std::lock_guard<std::mutex> lock(emotionMutex_);
                std::cout << "[当前情绪: " << currentEmotion_ << "] ";
            }
            std::cout << "你: " << std::flush;
            std::getline(std::cin, input);

            if (!running) break;

            if (input == "quit" || input == "exit") {
                std::cout << "……再见。" << std::endl;
                running = false;
                break;
            }
            if (input.empty()) continue;

            cancelled_.store(false);

            std::string emotion;
            {
                std::lock_guard<std::mutex> lock(emotionMutex_);
                emotion = currentEmotion_;
            }

            int aff = memory_.getAffinity();
            std::string emotionTrend = memory_.getEmotionTrend();
            std::string context = memory_.getContext(5);
            std::string prompt = buildPrompt(input, emotion, context, aff, emotionTrend, getPersonaPtr());

            std::cout << "（思考中...）" << std::endl;

            auto future = ai_.chatAsync(prompt);
            std::string reply;
            
            if (waitForAI(future, reply)) {
                if (reply.empty()) {
                    std::cout << "TA: ……（沉默）" << std::endl;
                } else {
                    std::cout << "TA: " << reply << std::endl;
                }
            } else {
                std::cout << "TA: ……（网络有点问题）" << std::endl;
            }

            memory_.saveChat(input, reply.empty() ? "……" : reply, emotion);
            memory_.recordEmotion(emotion);
            memory_.incrementChatCount();
            int delta = calculateAffinityDelta(emotion, input);
            memory_.updateAffinity(delta);
            std::cout << std::endl;
        }
    });

    if (cameraReady) {
        LOGI("Brain", "摄像头已启动，等待第一帧...");
        cv::Mat frame;
        int frameCount = 0;
        int emptyCount = 0;
        while (running && emptyCount < 300) {
            if (camera_.getFrame(frame)) {
                LOGI("Brain", "收到第一帧，摄像头正常");
                break;
            }
            emptyCount++;
            cv::waitKey(10);
        }
        if (emptyCount >= 300) {
            LOGW("Brain", "摄像头超时无数据，继续尝试...");
        }

        while (running) {
            if (camera_.getFrame(frame)) {
                if (frameCount % faceDetectInterval_ == 0) {
                    auto faces = faceDetector_.detect(frame);
                    for (const auto& face : faces) {
                        cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
                        cv::Mat faceROI = frame(face);
                        std::string emotion = emotionRecognizer_.recognize(faceROI);
                        cv::putText(frame, emotion, cv::Point(face.x, face.y - 10),
                                    cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
                        {
                            std::lock_guard<std::mutex> lock(emotionMutex_);
                            currentEmotion_ = emotion;
                        }
                    }
                }
                cv::imshow("AI Pet - Camera", frame);
                frameCount++;
            }
            int key = cv::waitKey(1);
            if (key == 'q' || key == 'Q') {
                running = false;
                break;
            }
        }
        cv::destroyAllWindows();
    } else {
        chatThread.join();
        camera_.close();
        return;
    }

    running = false;
    cancelAI();
    if (chatThread.joinable()) {
        chatThread.join();
    }
    camera_.close();
}

void Brain::runUIMode() {
    LOGI("Brain", "启动 UI 模式");

    const auto& cfg = config_.get();
    if (!ui_.init(cfg.ui_window_width, cfg.ui_window_height)) {
        LOGE("Brain", "UI 初始化失败");
        return;
    }

    bool visionReady = initVision();
    bool cameraReady = false;
    if (visionReady) {
        cameraReady = camera_.open();
    }
    if (!cameraReady) {
        LOGW("Brain", "摄像头未就绪，UI 将在无摄像头模式下运行");
    }

    if (memory_.open()) {
        LOGI("Brain", "记忆系统已启动");
    }

    loadPersona();

    std::atomic<bool> running{true};

    ui_.setSendCallback([this, &running](const std::string& input) {
        if (input == "quit" || input == "exit") {
            running = false;
            cancelAI();
            return;
        }

        if (aiRunning_.load()) {
            LOGW("Brain", "AI 正在处理中，请稍候");
            return;
        }

        LOGD("Brain", "收到用户输入: " + input);

        cancelled_.store(false);
        ui_.addChatMessage(input, true);
        ui_.setThinking(true);

        std::thread([this, input]() {
            try {
                std::string emotion;
                {
                    std::lock_guard<std::mutex> lock(emotionMutex_);
                    emotion = currentEmotion_;
                }

                int aff = memory_.getAffinity();
                std::string emotionTrend = memory_.getEmotionTrend();
                std::string context = memory_.getContext(5);
                
                LOGD("Brain", "构建 prompt (persona: " + std::string(getPersonaPtr() ? "是" : "否") + ")");
                std::string prompt = buildPrompt(input, emotion, context, aff, emotionTrend, getPersonaPtr());
                
                LOGD("Brain", "启动异步 AI 调用...");
                aiRunning_.store(true);
                
                auto future = ai_.chatAsync(prompt);
                std::string reply;
                
                if (waitForAI(future, reply, 180000)) {
                    LOGD("Brain", "AI 回复长度: " + std::to_string(reply.size()) + " 字节");

                    if (reply.empty()) {
                        LOGW("Brain", "AI 返回空回复");
                        reply = "……";
                    }

                    ui_.setThinking(false);
                    ui_.addChatMessage(reply, false);

                    memory_.saveChat(input, reply, emotion);
                    memory_.recordEmotion(emotion);
                    memory_.incrementChatCount();
                    int delta = calculateAffinityDelta(emotion, input);
                    memory_.updateAffinity(delta);

                    ui_.updateEmotion(emotion);
                    
                    LOGD("Brain", "消息处理完成");
                } else {
                    ui_.setThinking(false);
                    ui_.addChatMessage("……抱歉，网络好像有点问题", false);
                }
                
                aiRunning_.store(false);
            } catch (const std::exception& e) {
                LOGE("Brain", "处理消息异常: " + std::string(e.what()));
                ui_.setThinking(false);
                aiRunning_.store(false);
                ui_.addChatMessage("系统错误: " + std::string(e.what()), false);
            } catch (...) {
                LOGE("Brain", "处理消息发生未知异常");
                ui_.setThinking(false);
                aiRunning_.store(false);
                ui_.addChatMessage("系统错误", false);
            }
        }).detach();
    });

    std::thread visionThread;
    if (cameraReady) {
        visionThread = std::thread([this, &running]() {
            cv::Mat frame;
            int frameCount = 0;
            while (running) {
                if (camera_.getFrame(frame)) {
                    if (frameCount % faceDetectInterval_ == 0) {
                        auto faces = faceDetector_.detect(frame);
                        for (const auto& face : faces) {
                            cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
                            cv::Mat faceROI = frame(face);
                            std::string emotion = emotionRecognizer_.recognize(faceROI);
                            cv::putText(frame, emotion, cv::Point(face.x, face.y - 10),
                                        cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
                            {
                                std::lock_guard<std::mutex> lock(emotionMutex_);
                                currentEmotion_ = emotion;
                            }
                            ui_.updateEmotion(emotion);
                        }
                    }
                    ui_.updateCameraFrame(frame);
                    frameCount++;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        });
        LOGI("Brain", "摄像头后台线程已启动");
    }

    while (running) {
        if (!ui_.tick()) {
            running = false;
            break;
        }
    }

    running = false;
    cancelAI();
    if (visionThread.joinable()) {
        visionThread.join();
    }
    camera_.close();
    ui_.shutdown();
    LOGI("Brain", "UI 模式已退出");
}
