#include "controller/brain.h"
#include "ai/personality.h"
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>

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
}

void Brain::setPersonaName(const std::string& name) {
    personaName_ = name;
}

bool Brain::loadPersona() {
    const auto& personasDir = config_.get().personas_dir;
    if (!personaName_.empty()) {
        std::string dir = personasDir + "/" + personaName_;
        if (personaLoader_.load(dir)) {
            std::cout << "[Brain] 已加载角色: " << personaLoader_.getData().name << std::endl;
            return true;
        } else {
            std::cerr << "[Brain] 指定角色 '" << personaName_ << "' 加载失败" << std::endl;
            return false;
        }
    } else {
        if (personaLoader_.autoLoad(personasDir)) {
            std::cout << "[Brain] 已加载角色: " << personaLoader_.getData().name << std::endl;
            return true;
        } else {
            std::cout << "[Brain] 未找到角色文件，使用默认人格" << std::endl;
            return false;
        }
    }
}

const PersonaData* Brain::getPersonaPtr() const {
    return personaLoader_.isLoaded() ? &personaLoader_.getData() : nullptr;
}

bool Brain::initVision() {
    if (!faceDetector_.load()) {
        std::cerr << "[Brain] 人脸检测模型加载失败" << std::endl;
        return false;
    }
    if (!emotionRecognizer_.load()) {
        std::cerr << "[Brain] 表情识别模型加载失败" << std::endl;
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

    // 初始化记忆系统
    if (memory_.open()) {
        std::cout << "[Brain] 记忆系统已启动" << std::endl;
    }

    // 加载角色文件
    loadPersona();

    // 显示当前关系状态
    int affinity = memory_.getAffinity();
    AffinityLevel level = getAffinityLevel(affinity);
    std::cout << "[关系] 好感度: " << affinity << "/100 (" << getAffinityLevelName(level) << ")" << std::endl;

    std::string input;
    while (true) {
        std::cout << "[" << getAffinityLevelName(getAffinityLevel(memory_.getAffinity())) << " " << memory_.getAffinity() << "] 你: ";
        std::getline(std::cin, input);

        // 退出条件
        if (input == "quit" || input == "exit" || input.empty()) {
            if (input == "quit" || input == "exit") {
                std::cout << "……再见。" << std::endl;
                break;
            }
            continue;
        }

        // 获取关系数据
        int aff = memory_.getAffinity();
        std::string emotionTrend = memory_.getEmotionTrend();
        std::string context = memory_.getContext(5);

        // 构造 prompt（融合人格进化 + 情绪策略）
        std::string prompt = buildPrompt(input, currentEmotion_, context, aff, emotionTrend, getPersonaPtr());

        // 调用 AI
        std::cout << "（思考中...）" << std::endl;
        std::string reply = ai_.chat(prompt);

        if (reply.empty()) {
            std::cout << "TA: ……（沉默）" << std::endl;
        } else {
            std::cout << "TA: " << reply << std::endl;
        }

        // 保存对话 + 更新关系
        memory_.saveChat(input, reply.empty() ? "……" : reply, currentEmotion_);
        memory_.recordEmotion(currentEmotion_);
        memory_.incrementChatCount();
        int delta = calculateAffinityDelta(currentEmotion_, input);
        memory_.updateAffinity(delta);

        std::cout << std::endl;
    }
}

void Brain::runCameraMode() {
    std::cout << "[Brain] 启动摄像头测试模式..." << std::endl;

    if (!initVision()) {
        std::cerr << "[Brain] 视觉模块初始化失败" << std::endl;
        return;
    }

    if (!camera_.open()) {
        std::cerr << "[Brain] 无法打开摄像头，请检查 /tmp/yuyv.sdp 是否存在" << std::endl;
        return;
    }

    std::cout << "[Brain] 摄像头已打开，按 'q' 退出" << std::endl;

    cv::Mat frame;
    while (true) {
        if (camera_.getFrame(frame)) {
            // 人脸检测
            auto faces = faceDetector_.detect(frame);

            for (const auto& face : faces) {
                // 绘制人脸框
                cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);

                // 表情识别
                cv::Mat faceROI = frame(face);
                std::string emotion = emotionRecognizer_.recognize(faceROI);

                // 显示情绪标签
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

    // 初始化视觉模块
    bool visionReady = initVision();
    bool cameraReady = false;
    if (visionReady) {
        cameraReady = camera_.open();
    }

    if (!cameraReady) {
        std::cout << "[Brain] 摄像头未就绪，将在纯对话模式下运行（无表情识别）" << std::endl;
    }

    // 初始化记忆系统
    if (memory_.open()) {
        std::cout << "[Brain] 记忆系统已启动" << std::endl;
    }

    // 加载角色文件
    loadPersona();

    std::atomic<bool> running{true};
    std::mutex emotionMutex;

    std::thread chatThread([&]() {
        std::string input;
        while (running) {
            {
                std::lock_guard<std::mutex> lock(emotionMutex);
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

            std::string emotion;
            {
                std::lock_guard<std::mutex> lock(emotionMutex);
                emotion = currentEmotion_;
            }

            int aff = memory_.getAffinity();
            std::string emotionTrend = memory_.getEmotionTrend();
            std::string context = memory_.getContext(5);
            std::string prompt = buildPrompt(input, emotion, context, aff, emotionTrend, getPersonaPtr());

            std::cout << "（思考中...）" << std::endl;
            std::string reply = ai_.chat(prompt);

            if (reply.empty()) {
                std::cout << "TA: ……（沉默）" << std::endl;
            } else {
                std::cout << "TA: " << reply << std::endl;
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
        std::cout << "[Brain] 摄像头已启动，等待第一帧..." << std::endl;
        cv::Mat frame;
        int frameCount = 0;
        int emptyCount = 0;
        while (running && emptyCount < 300) {
            if (camera_.getFrame(frame)) {
                std::cout << "[Brain] 收到第一帧，摄像头正常" << std::endl;
                break;
            }
            emptyCount++;
            cv::waitKey(10);
        }
        if (emptyCount >= 300) {
            std::cout << "[Brain] 摄像头超时无数据，继续尝试..." << std::endl;
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
                            std::lock_guard<std::mutex> lock(emotionMutex);
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
        // 无摄像头时，主线程等待对话线程结束
        chatThread.join();
        camera_.close();
        return;
    }

    running = false;
    if (chatThread.joinable()) {
        chatThread.join();
    }
    camera_.close();
}

void Brain::runUIMode() {
    std::cout << "[Brain] 启动 UI 模式..." << std::endl;

    const auto& cfg = config_.get();
    if (!ui_.init(cfg.ui_window_width, cfg.ui_window_height)) {
        std::cerr << "[Brain] UI 初始化失败" << std::endl;
        return;
    }

    // 初始化视觉模块
    bool visionReady = initVision();
    bool cameraReady = false;
    if (visionReady) {
        cameraReady = camera_.open();
    }
    if (!cameraReady) {
        std::cout << "[Brain] 摄像头未就绪，UI 将在无摄像头模式下运行" << std::endl;
    }

    // 初始化记忆系统
    if (memory_.open()) {
        std::cout << "[Brain] 记忆系统已启动" << std::endl;
    }

    // 加载角色文件
    loadPersona();

    std::atomic<bool> running{true};
    std::mutex emotionMutex;

    // 设置发送回调：用户点击发送或按回车时触发
    ui_.setSendCallback([this, &running, &emotionMutex](const std::string& input) {
        if (input == "quit" || input == "exit") {
            running = false;
            return;
        }

        std::cout << "[Brain] 收到用户输入: " << input << std::endl;

        // 显示用户消息
        ui_.addChatMessage(input, true);
        ui_.setThinking(true);

        // 在后台线程调用 AI（避免阻塞 UI）
        std::thread([this, &running, &emotionMutex, input]() {
            try {
                std::string emotion;
                {
                    std::lock_guard<std::mutex> lock(emotionMutex);
                    emotion = currentEmotion_;
                }

                int aff = memory_.getAffinity();
                std::string emotionTrend = memory_.getEmotionTrend();
                std::string context = memory_.getContext(5);
                
                std::cout << "[Brain] 构建 prompt (persona: " << (getPersonaPtr() ? "是" : "否") << ")..." << std::endl;
                std::string prompt = buildPrompt(input, emotion, context, aff, emotionTrend, getPersonaPtr());
                
                std::cout << "[Brain] 调用 AI..." << std::endl;
                std::string reply = ai_.chat(prompt);

                std::cout << "[Brain] AI 回复长度: " << reply.size() << " 字节" << std::endl;

                if (reply.empty()) {
                    std::cout << "[Brain] AI 返回空回复，使用默认回复" << std::endl;
                    reply = "......";
                }

                // 更新 UI
                ui_.setThinking(false);
                ui_.addChatMessage(reply, false);

                // 保存记忆 + 更新关系
                memory_.saveChat(input, reply, emotion);
                memory_.recordEmotion(emotion);
                memory_.incrementChatCount();
                int delta = calculateAffinityDelta(emotion, input);
                memory_.updateAffinity(delta);

                // 更新 UI 情绪显示
                ui_.updateEmotion(emotion);
                
                std::cout << "[Brain] 消息处理完成" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[Brain] 处理消息异常: " << e.what() << std::endl;
                ui_.setThinking(false);
                ui_.addChatMessage("系统错误: " + std::string(e.what()), false);
            } catch (...) {
                std::cerr << "[Brain] 处理消息发生未知异常" << std::endl;
                ui_.setThinking(false);
                ui_.addChatMessage("系统错误", false);
            }
        }).detach();
    });

    // 后台线程：摄像头读取 + 人脸检测 + 表情识别
    std::thread visionThread;
    if (cameraReady) {
        visionThread = std::thread([&]() {
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
                                std::lock_guard<std::mutex> lock(emotionMutex);
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
        std::cout << "[Brain] 摄像头后台线程已启动" << std::endl;
    }

    // 主线程：LVGL + SDL 事件循环
    while (running) {
        if (!ui_.tick()) {
            running = false;
            break;
        }
    }

    running = false;
    if (visionThread.joinable()) {
        visionThread.join();
    }
    camera_.close();
    ui_.shutdown();
    std::cout << "[Brain] UI 模式已退出" << std::endl;
}
