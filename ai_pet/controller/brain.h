#pragma once

#include "vision/camera.h"
#include "vision/face.h"
#include "vision/emotion.h"
#include "ai/gemma.h"
#include "ai/prompt.h"
#include "ai/persona_loader.h"
#include "memory/sqlite.h"
#include "ui/ui.h"
#include "config/config.h"
#include <string>

class Brain {
public:
    explicit Brain(const std::string& configPath = "config.json");
    ~Brain() = default;

    // 纯对话模式（终端交互 + 记忆系统）
    void runChatMode();

    // 摄像头测试模式（验证视频流 + 人脸检测 + 表情识别）
    void runCameraMode();

    // 完整模式（摄像头 + 表情识别 + 对话 + 记忆）
    void runFullMode();

    // UI 模式（LVGL + SDL 窗口）
    void runUIMode();

    // 指定要加载的角色名称（空字符串=自动检测）
    void setPersonaName(const std::string& name);

private:
    ConfigManager& config_;
    Camera camera_;
    FaceDetector faceDetector_;
    EmotionRecognizer emotionRecognizer_;
    GemmaAI ai_;
    MemoryDB memory_;
    UIManager ui_;
    PersonaLoader personaLoader_;
    std::string currentEmotion_;
    std::string personaName_;  // 指定的角色名称
    int faceDetectInterval_;
    int emotionStableThreshold_;

    // 初始化视觉模块
    bool initVision();

    // 加载角色文件
    bool loadPersona();

    // 获取 persona 指针（用于传递给 buildPrompt）
    const PersonaData* getPersonaPtr() const;
};
