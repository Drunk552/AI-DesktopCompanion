#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <functional>
#include <opencv2/opencv.hpp>
#include "lvgl.h"

// 对话消息结构
struct ChatMessage {
    std::string text;
    bool isUser;  // true=用户, false=AI
};

// UI 管理器 - 基于 LVGL + SDL
class UIManager {
public:
    static constexpr int MAX_CHAT_BUBBLES = 50; // 聊天气泡上限

    UIManager() = default;
    ~UIManager();

    // 初始化 LVGL + SDL 窗口
    bool init(int width = 480, int height = 320);

    // 主循环（必须在主线程调用）
    bool tick();

    // 关闭
    void shutdown();

    // === 线程安全的数据更新接口 ===
    void updateCameraFrame(const cv::Mat& frame);
    void updateEmotion(const std::string& emotion);
    void addChatMessage(const std::string& text, bool isUser);
    void setThinking(bool thinking);
    void setSendCallback(std::function<void(const std::string&)> cb);

    bool isReady() const { return initialized_; }

private:
    // 布局创建
    void createLayout();
    void createLeftPanel(lv_obj_t* parent);
    void createRightPanel(lv_obj_t* parent);

    // 回调
    static void sendBtnCallback(lv_event_t* e);
    static void textareaCallback(lv_event_t* e);

    // 内部更新
    void processUpdates();
    void renderCameraFrame();
    void trimChatBubbles();

    bool initialized_ = false;
    int winWidth_ = 480;
    int winHeight_ = 320;

    // LVGL
    lv_display_t* display_ = nullptr;
    lv_group_t* inputGroup_ = nullptr;

    // 左侧面板
    lv_obj_t* leftPanel_ = nullptr;
    lv_obj_t* petImage_ = nullptr;       // 宠物形象占位
    lv_obj_t* cameraCanvas_ = nullptr;   // 悬浮摄像头

    // 右侧面板
    lv_obj_t* rightPanel_ = nullptr;
    lv_obj_t* emotionTag_ = nullptr;     // 情绪胶囊标签
    lv_obj_t* chatPanel_ = nullptr;      // 聊天滚动区
    lv_obj_t* inputArea_ = nullptr;      // 输入框
    lv_obj_t* sendBtn_ = nullptr;        // 发送按钮
    lv_obj_t* thinkingLabel_ = nullptr;  // 思考中标签

    // 摄像头帧（线程安全）
    std::mutex frameMutex_;
    cv::Mat latestFrame_;
    bool frameUpdated_ = false;

    // 摄像头 canvas 缓冲区
    uint8_t* canvasBuf_ = nullptr;
    int camW_ = 80;
    int camH_ = 60;

    // 情绪更新（线程安全）
    std::mutex emotionMutex_;
    std::string pendingEmotion_;
    bool emotionUpdated_ = false;

    // 消息队列（线程安全）
    std::mutex msgMutex_;
    std::vector<ChatMessage> pendingMessages_;
    bool thinkingState_ = false;
    bool thinkingUpdated_ = false;
    int bubbleCount_ = 0;

    // 字体
    lv_font_t* fontCN14_ = nullptr;
    lv_font_t* fontCN16_ = nullptr;
    std::vector<uint8_t> fontData_;

    // 发送回调
    std::function<void(const std::string&)> sendCallback_;
};
