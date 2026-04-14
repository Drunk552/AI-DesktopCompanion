/**
 * @file controller/brain.h
 * @brief AI 桌面宠物核心控制器
 * 
 * 作为中央协调器，协调视觉感知、情绪识别、AI对话、
 * 记忆存储和 UI 显示等模块。
 */

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
#include <future>
#include <atomic>
#include <mutex>

/**
 * @class Brain
 * @brief AI 桌面宠物核心控制器
 * @details 协调各子模块，提供多种运行模式
 */
class Brain {
public:
    /**
     * @brief 构造函数
     * @param configPath 配置文件路径
     */
    explicit Brain(const std::string& configPath = "config.json");
    
    /**
     * @brief 析构函数
     */
    ~Brain();
    
    /**
     * @brief 运行纯对话模式
     * @details 终端交互 + 记忆系统，无 UI 和摄像头
     */
    void runChatMode();
    
    /**
     * @brief 运行摄像头测试模式
     * @details 验证视频流、人脸检测和表情识别
     */
    void runCameraMode();
    
    /**
     * @brief 运行完整模式
     * @details 摄像头 + 表情识别 + 对话 + 记忆
     */
    void runFullMode();
    
    /**
     * @brief 运行 UI 模式
     * @details LVGL + SDL 窗口界面
     */
    void runUIMode();
    
    /**
     * @brief 设置要加载的角色名称
     * @param name 角色名称，空字符串表示自动检测
     */
    void setPersonaName(const std::string& name);

private:
    /**
     * @brief 初始化视觉模块
     * @return bool 初始化是否成功
     */
    bool initVision();
    
    /**
     * @brief 加载角色文件
     * @return bool 加载是否成功
     */
    bool loadPersona();
    
    /**
     * @brief 获取角色数据指针
     * @return const PersonaData* 角色数据指针
     */
    const PersonaData* getPersonaPtr() const;
    
    /**
     * @brief 取消正在进行的 AI 请求
     */
    void cancelAI();
    
    /**
     * @brief 等待 AI 异步请求完成
     * @param future 异步future引用
     * @param result 输出参数，接收结果
     * @param timeoutMs 超时毫秒数
     * @return bool 是否成功获取结果
     */
    bool waitForAI(std::future<std::string>& future, std::string& result, int timeoutMs = 120000);
    
    ConfigManager& config_;              ///< 配置管理器引用
    Camera camera_;                      ///< 摄像头模块
    FaceDetector faceDetector_;          ///< 人脸检测器
    EmotionRecognizer emotionRecognizer_; ///< 表情识别器
    GemmaAI ai_;                         ///< AI 对话接口
    MemoryDB memory_;                    ///< 记忆数据库
    UIManager ui_;                       ///< UI 管理器
    PersonaLoader personaLoader_;         ///< 角色加载器
    std::string currentEmotion_;         ///< 当前检测到的情绪
    std::string personaName_;            ///< 指定加载的角色名称
    int faceDetectInterval_;             ///< 人脸检测间隔
    int emotionStableThreshold_;         ///< 情绪稳定阈值
    
    std::atomic<bool> aiRunning_{false};    ///< AI 是否正在运行
    std::atomic<bool> cancelled_{false};    ///< 是否已取消
    std::mutex emotionMutex_;               ///< 情绪数据互斥锁
};
