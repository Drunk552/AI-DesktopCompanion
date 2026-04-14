/**
 * @file ui/ui.h
 * @brief UI 界面管理模块
 *
 * 基于 LVGL + SDL 的桌面宠物界面。
 * 支持聊天显示、摄像头画中画、情绪状态显示等功能。
 */

#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <functional>
#include <opencv2/opencv.hpp>
#include "lvgl.h"

/**
 * @struct ChatMessage
 * @brief 对话消息结构
 */
struct ChatMessage {
    std::string text;  ///< 消息文本
    bool isUser;       ///< true=用户消息, false=AI消息
};

/**
 * @class UIManager
 * @brief UI 管理器
 * @details 基于 LVGL + SDL 实现桌面宠物界面
 */
class UIManager {
public:
    static constexpr int MAX_CHAT_BUBBLES = 50;  ///< 聊天气泡最大数量

    /**
     * @brief 构造函数
     */
    UIManager() = default;

    /**
     * @brief 析构函数
     */
    ~UIManager();

    /**
     * @brief 初始化 UI
     * @param width 窗口宽度
     * @param height 窗口高度
     * @return bool 初始化是否成功
     */
    bool init(int width = 480, int height = 320);

    /**
     * @brief 主循环
     * @return bool 是否继续运行
     * @note 必须在主线程调用
     */
    bool tick();

    /**
     * @brief 关闭 UI
     */
    void shutdown();

    // ==================== 线程安全的数据更新接口 ====================

    /**
     * @brief 更新摄像头画面
     * @param frame 摄像头帧
     */
    void updateCameraFrame(const cv::Mat& frame);

    /**
     * @brief 更新情绪显示
     * @param emotion 情绪字符串
     */
    void updateEmotion(const std::string& emotion);

    /**
     * @brief 添加聊天消息
     * @param text 消息文本
     * @param isUser 是否为用户消息
     */
    void addChatMessage(const std::string& text, bool isUser);

    /**
     * @brief 设置思考状态
     * @param thinking 是否正在思考
     */
    void setThinking(bool thinking);

    /**
     * @brief 设置消息发送回调
     * @param cb 回调函数
     */
    void setSendCallback(std::function<void(const std::string&)> cb);

    /**
     * @brief 检查 UI 是否就绪
     * @return bool 是否已初始化
     */
    bool isReady() const { return initialized_; }

private:
    /**
     * @brief 创建 UI 布局
     */
    void createLayout();

    /**
     * @brief 创建左侧面板
     * @param parent 父对象
     */
    void createLeftPanel(lv_obj_t* parent);

    /**
     * @brief 创建右侧面板
     * @param parent 父对象
     */
    void createRightPanel(lv_obj_t* parent);

    /**
     * @brief 发送按钮回调
     */
    static void sendBtnCallback(lv_event_t* e);

    /**
     * @brief 输入框回调
     */
    static void textareaCallback(lv_event_t* e);

    /**
     * @brief 处理待处理的更新
     */
    void processUpdates();

    /**
     * @brief 渲染摄像头画面
     */
    void renderCameraFrame();

    /**
     * @brief 清理多余的聊天气泡
     */
    void trimChatBubbles();

    bool initialized_ = false;   ///< 是否已初始化
    int winWidth_ = 480;         ///< 窗口宽度
    int winHeight_ = 320;        ///< 窗口高度

    lv_display_t* display_ = nullptr;   ///< LVGL 显示
    lv_group_t* inputGroup_ = nullptr;  ///< LVGL 输入组

    lv_obj_t* leftPanel_ = nullptr;     ///< 左侧面板
    lv_obj_t* petImage_ = nullptr;      ///< 宠物形象占位
    lv_obj_t* cameraCanvas_ = nullptr;  ///< 悬浮摄像头画布

    lv_obj_t* rightPanel_ = nullptr;     ///< 右侧面板
    lv_obj_t* emotionTag_ = nullptr;     ///< 情绪标签
    lv_obj_t* chatPanel_ = nullptr;      ///< 聊天区域
    lv_obj_t* inputArea_ = nullptr;      ///< 输入框
    lv_obj_t* sendBtn_ = nullptr;        ///< 发送按钮
    lv_obj_t* thinkingLabel_ = nullptr;  ///< 思考中标签

    std::mutex frameMutex_;  ///< 帧数据互斥锁
    cv::Mat latestFrame_;    ///< 最新帧
    bool frameUpdated_ = false;  ///< 帧更新标志

    std::vector<uint8_t> canvasBuf_;  ///< 摄像头画布缓冲区
    int camW_ = 80;                  ///< 摄像头画布宽度
    int camH_ = 60;                  ///< 摄像头画布高度

    std::mutex emotionMutex_;    ///< 情绪数据互斥锁
    std::string pendingEmotion_; ///< 待处理的情绪
    bool emotionUpdated_ = false;  ///< 情绪更新标志

    std::mutex msgMutex_;                   ///< 消息队列互斥锁
    std::vector<ChatMessage> pendingMessages_;  ///< 待处理消息
    bool thinkingState_ = false;           ///< 思考状态
    bool thinkingUpdated_ = false;         ///< 思考状态更新标志
    int bubbleCount_ = 0;                  ///< 气泡计数

    lv_font_t* fontCN14_ = nullptr;  ///< 中文字体 14px
    lv_font_t* fontCN16_ = nullptr;  ///< 中文字体 16px
    std::vector<uint8_t> fontData_;  ///< 字体数据

    std::function<void(const std::string&)> sendCallback_;  ///< 发送回调
};
