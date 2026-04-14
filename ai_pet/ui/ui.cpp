#include "ui/ui.h"
#include "ui/assets/pet/white/idle/dog_white_idle.h"
#include "logger/logger.h"
#include <cstring>
#include <fstream>
#include <vector>
#include <SDL2/SDL.h>

#ifndef APP_ASSET_DIR
#define APP_ASSET_DIR "ui/assets"
#endif

namespace {

const void* kDogWhiteIdleFrames[] = {
    &dog_white_idle_0,
    &dog_white_idle_1,
    &dog_white_idle_2,
    &dog_white_idle_3,
};

constexpr uint32_t kDogWhiteIdleFrameCount = sizeof(kDogWhiteIdleFrames) / sizeof(kDogWhiteIdleFrames[0]);

}

UIManager::~UIManager() {
    shutdown();
}

bool UIManager::init(int width, int height) {
    if (initialized_) return true;

    winWidth_ = width;
    winHeight_ = height;

    lv_init();

    display_ = lv_sdl_window_create(winWidth_, winHeight_);
    if (!display_) {
        LOGE("UI", "无法创建 SDL 窗口");
        return false;
    }

    lv_sdl_mouse_create();

    lv_indev_t* kb = lv_sdl_keyboard_create();
    inputGroup_ = lv_group_create();
    lv_indev_set_group(kb, inputGroup_);
    lv_group_set_default(inputGroup_);

    canvasBuf_.resize(camW_ * camH_ * 4, 0);

    const char* fontPath = "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc";
    std::ifstream fontFile(fontPath, std::ios::binary | std::ios::ate);
    if (fontFile.is_open()) {
        size_t fileSize = fontFile.tellg();
        fontFile.seekg(0, std::ios::beg);
        fontData_.resize(fileSize);
        fontFile.read(reinterpret_cast<char*>(fontData_.data()), fileSize);
        fontFile.close();
        fontCN14_ = lv_tiny_ttf_create_data(fontData_.data(), fontData_.size(), 14);
        fontCN16_ = lv_tiny_ttf_create_data(fontData_.data(), fontData_.size(), 16);
        LOGI("UI", "中文字体已加载 (" + std::to_string(fileSize / 1024 / 1024) + "MB)");
    }
    if (!fontCN14_ || !fontCN16_) {
        LOGW("UI", "无法加载中文字体，回退到默认字体");
        fontCN14_ = const_cast<lv_font_t*>(&lv_font_montserrat_14);
        fontCN16_ = const_cast<lv_font_t*>(&lv_font_montserrat_16);
    }

    createLayout();

    initialized_ = true;
    LOGI("UI", "LVGL + SDL 窗口已创建 (" + std::to_string(winWidth_) + "x" + std::to_string(winHeight_) + ")");
    return true;
}

bool UIManager::tick() {
    if (!initialized_) return false;
    processUpdates();
    lv_tick_inc(5);
    uint32_t ms = lv_timer_handler();
    SDL_Delay(ms < 5 ? 5 : ms);
    return true;
}

void UIManager::shutdown() {
    if (!initialized_) return;
    canvasBuf_.clear();
    initialized_ = false;
    LOGI("UI", "窗口已关闭");
}

// ============================================================
// UI 布局创建
// ============================================================

void UIManager::createLayout() {
    lv_obj_t* scr = lv_screen_active();

    // 主屏幕样式
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x121218), 0);
    lv_obj_set_style_pad_all(scr, 0, 0);

    // 主容器：Flex Row 左右分栏
    lv_obj_t* mainRow = lv_obj_create(scr);
    lv_obj_set_size(mainRow, winWidth_, winHeight_);
    lv_obj_set_pos(mainRow, 0, 0);
    lv_obj_set_style_bg_opa(mainRow, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(mainRow, 0, 0);
    lv_obj_set_style_pad_all(mainRow, 0, 0);
    lv_obj_set_style_pad_column(mainRow, 0, 0);
    lv_obj_set_flex_flow(mainRow, LV_FLEX_FLOW_ROW);
    lv_obj_clear_flag(mainRow, LV_OBJ_FLAG_SCROLLABLE);

    createLeftPanel(mainRow);
    createRightPanel(mainRow);
}

// ============================================================
// 左侧面板：宠物展示 + 悬浮摄像头
// ============================================================

void UIManager::createLeftPanel(lv_obj_t* parent) {
    int panelW = 192; // 40% of 480

    leftPanel_ = lv_obj_create(parent);
    lv_obj_set_size(leftPanel_, panelW, winHeight_);
    lv_obj_set_style_bg_color(leftPanel_, lv_color_hex(0x0D0D14), 0);
    lv_obj_set_style_border_width(leftPanel_, 0, 0);
    lv_obj_set_style_radius(leftPanel_, 0, 0);
    lv_obj_set_style_pad_all(leftPanel_, 0, 0);
    lv_obj_clear_flag(leftPanel_, LV_OBJ_FLAG_SCROLLABLE);

    // 左侧默认背景图（最底层）
    lv_obj_t* leftBgImage = lv_image_create(leftPanel_);
    lv_image_set_src(leftBgImage, "/" APP_ASSET_DIR "/backgrounds/left/default.png");
    lv_obj_set_size(leftBgImage, panelW, winHeight_);
    lv_obj_set_pos(leftBgImage, 0, 0);
    lv_obj_move_to_index(leftBgImage, 0);

    // 宠物待机动画（居中）
    petImage_ = lv_animimg_create(leftPanel_);
    lv_animimg_set_src(petImage_, kDogWhiteIdleFrames, kDogWhiteIdleFrameCount);
    lv_animimg_set_duration(petImage_, 800);
    lv_animimg_set_repeat_count(petImage_, LV_ANIM_REPEAT_INFINITE);
    lv_obj_center(petImage_);
    lv_animimg_start(petImage_);

    // 宠物名字标签（底部）
    lv_obj_t* nameLabel = lv_label_create(leftPanel_);
    lv_label_set_text(nameLabel, "AI Pet");
    lv_obj_set_style_text_color(nameLabel, lv_color_hex(0x6C7086), 0);
    lv_obj_set_style_text_font(nameLabel, fontCN14_, 0);
    lv_obj_align(nameLabel, LV_ALIGN_BOTTOM_MID, 0, -12);

    // 悬浮摄像头画中画（右上角，绝对定位）
    cameraCanvas_ = lv_canvas_create(leftPanel_);
    lv_canvas_set_buffer(cameraCanvas_, canvasBuf_.data(), camW_, camH_, LV_COLOR_FORMAT_ARGB8888);
    lv_obj_set_style_radius(cameraCanvas_, 12, 0);
    lv_obj_set_style_clip_corner(cameraCanvas_, true, 0);
    lv_obj_set_style_shadow_width(cameraCanvas_, 12, 0);
    lv_obj_set_style_shadow_color(cameraCanvas_, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_opa(cameraCanvas_, LV_OPA_60, 0);
    lv_obj_set_style_border_width(cameraCanvas_, 1, 0);
    lv_obj_set_style_border_color(cameraCanvas_, lv_color_hex(0x313244), 0);
    lv_obj_align(cameraCanvas_, LV_ALIGN_TOP_RIGHT, -8, 8);
}

// ============================================================
// 右侧面板：状态栏 + 聊天 + 输入
// ============================================================

void UIManager::createRightPanel(lv_obj_t* parent) {
    int panelW = 288; // 60% of 480

    rightPanel_ = lv_obj_create(parent);
    lv_obj_set_size(rightPanel_, panelW, winHeight_);
    lv_obj_set_style_bg_color(rightPanel_, lv_color_hex(0x1E1E1E), 0);
    lv_obj_set_style_bg_opa(rightPanel_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(rightPanel_, 0, 0);
    lv_obj_set_style_radius(rightPanel_, 0, 0);
    lv_obj_set_style_pad_all(rightPanel_, 0, 0);
    lv_obj_set_style_pad_row(rightPanel_, 0, 0);
    lv_obj_set_flex_flow(rightPanel_, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(rightPanel_, LV_OBJ_FLAG_SCROLLABLE);

    // 右侧默认背景图（最底层，不参与 Flex 布局）
    lv_obj_t* rightBgImage = lv_image_create(rightPanel_);
    lv_image_set_src(rightBgImage, "/" APP_ASSET_DIR "/backgrounds/right/default.png");
    lv_obj_set_size(rightBgImage, panelW, winHeight_);
    lv_obj_set_pos(rightBgImage, 0, 0);
    lv_obj_add_flag(rightBgImage, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_move_to_index(rightBgImage, 0);

    // ===== 3.2.1 顶部状态栏 (top_bar) =====
    lv_obj_t* topBar = lv_obj_create(rightPanel_);
    lv_obj_set_size(topBar, panelW, 32);
    lv_obj_set_style_bg_color(topBar, lv_color_hex(0x181818), 0);
    lv_obj_set_style_bg_opa(topBar, LV_OPA_80, 0);
    lv_obj_set_style_border_width(topBar, 0, 0);
    lv_obj_set_style_border_side(topBar, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_color(topBar, lv_color_hex(0x2A2A2A), 0);
    lv_obj_set_style_radius(topBar, 0, 0);
    lv_obj_set_style_pad_left(topBar, 8, 0);
    lv_obj_set_style_pad_right(topBar, 8, 0);
    lv_obj_set_style_pad_top(topBar, 4, 0);
    lv_obj_set_style_pad_bottom(topBar, 4, 0);
    lv_obj_clear_flag(topBar, LV_OBJ_FLAG_SCROLLABLE);

    // 情绪胶囊标签（左侧）
    emotionTag_ = lv_obj_create(topBar);
    lv_obj_set_size(emotionTag_, LV_SIZE_CONTENT, 22);
    lv_obj_set_style_bg_color(emotionTag_, lv_color_hex(0x2D6A4F), 0);
    lv_obj_set_style_radius(emotionTag_, 11, 0);
    lv_obj_set_style_pad_left(emotionTag_, 10, 0);
    lv_obj_set_style_pad_right(emotionTag_, 10, 0);
    lv_obj_set_style_pad_top(emotionTag_, 2, 0);
    lv_obj_set_style_pad_bottom(emotionTag_, 2, 0);
    lv_obj_set_style_border_width(emotionTag_, 0, 0);
    lv_obj_clear_flag(emotionTag_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(emotionTag_, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t* emotionText = lv_label_create(emotionTag_);
    lv_label_set_text(emotionText, "\xE2\x98\xBA \u5E73\u9759");  // ☺ 平静
    lv_obj_set_style_text_color(emotionText, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(emotionText, fontCN14_, 0);
    lv_obj_center(emotionText);

    // 标题（右侧）
    lv_obj_t* titleLabel = lv_label_create(topBar);
    lv_label_set_text(titleLabel, "AI Pet");
    lv_obj_set_style_text_color(titleLabel, lv_color_hex(0x6C7086), 0);
    lv_obj_set_style_text_font(titleLabel, fontCN14_, 0);
    lv_obj_align(titleLabel, LV_ALIGN_RIGHT_MID, 0, 0);

    // ===== 3.2.2 聊天消息区 (chat_list) =====
    chatPanel_ = lv_obj_create(rightPanel_);
    lv_obj_set_width(chatPanel_, panelW);
    lv_obj_set_flex_grow(chatPanel_, 1);
    lv_obj_set_style_bg_color(chatPanel_, lv_color_hex(0x101014), 0);
    lv_obj_set_style_bg_opa(chatPanel_, LV_OPA_30, 0);
    lv_obj_set_style_border_width(chatPanel_, 0, 0);
    lv_obj_set_style_radius(chatPanel_, 0, 0);
    lv_obj_set_style_pad_left(chatPanel_, 8, 0);
    lv_obj_set_style_pad_right(chatPanel_, 8, 0);
    lv_obj_set_style_pad_top(chatPanel_, 6, 0);
    lv_obj_set_style_pad_bottom(chatPanel_, 6, 0);
    lv_obj_set_flex_flow(chatPanel_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(chatPanel_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(chatPanel_, 6, 0);
    lv_obj_set_scroll_dir(chatPanel_, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(chatPanel_, LV_SCROLLBAR_MODE_AUTO);

    // 思考中标签
    thinkingLabel_ = lv_label_create(chatPanel_);
    lv_label_set_text(thinkingLabel_, "");
    lv_obj_set_style_text_color(thinkingLabel_, lv_color_hex(0x94E2D5), 0);
    lv_obj_set_style_text_font(thinkingLabel_, fontCN14_, 0);
    lv_obj_add_flag(thinkingLabel_, LV_OBJ_FLAG_HIDDEN);

    // ===== 3.2.3 底部输入区 (bottom_bar) =====
    lv_obj_t* bottomBar = lv_obj_create(rightPanel_);
    lv_obj_set_size(bottomBar, panelW, 40);
    lv_obj_set_style_bg_color(bottomBar, lv_color_hex(0x181818), 0);
    lv_obj_set_style_bg_opa(bottomBar, LV_OPA_90, 0);
    lv_obj_set_style_border_width(bottomBar, 0, 0);
    lv_obj_set_style_border_side(bottomBar, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_border_color(bottomBar, lv_color_hex(0x2A2A2A), 0);
    lv_obj_set_style_radius(bottomBar, 0, 0);
    lv_obj_set_style_pad_left(bottomBar, 6, 0);
    lv_obj_set_style_pad_right(bottomBar, 6, 0);
    lv_obj_set_style_pad_top(bottomBar, 4, 0);
    lv_obj_set_style_pad_bottom(bottomBar, 4, 0);
    lv_obj_set_style_pad_column(bottomBar, 6, 0);
    lv_obj_set_flex_flow(bottomBar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bottomBar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(bottomBar, LV_OBJ_FLAG_SCROLLABLE);

    // 输入框（flex grow 占据剩余空间）
    inputArea_ = lv_textarea_create(bottomBar);
    lv_obj_set_height(inputArea_, 30);
    lv_obj_set_flex_grow(inputArea_, 1);
    lv_textarea_set_placeholder_text(inputArea_, "输入消息...");
    lv_textarea_set_one_line(inputArea_, true);
    lv_obj_set_style_bg_color(inputArea_, lv_color_hex(0x2A2A2A), 0);
    lv_obj_set_style_bg_opa(inputArea_, LV_OPA_90, 0);
    lv_obj_set_style_text_color(inputArea_, lv_color_hex(0xE0E0E0), 0);
    lv_obj_set_style_border_width(inputArea_, 0, 0);
    lv_obj_set_style_radius(inputArea_, 15, 0);
    lv_obj_set_style_text_font(inputArea_, fontCN14_, 0);
    lv_obj_set_style_pad_left(inputArea_, 12, 0);
    lv_obj_set_style_pad_right(inputArea_, 12, 0);
    lv_obj_add_event_cb(inputArea_, textareaCallback, LV_EVENT_READY, this);
    if (inputGroup_) {
        lv_group_add_obj(inputGroup_, inputArea_);
        lv_group_focus_obj(inputArea_);
    }

    // 发送按钮（32x32 正圆，纸飞机符号）
    sendBtn_ = lv_button_create(bottomBar);
    lv_obj_set_size(sendBtn_, 32, 32);
    lv_obj_set_style_bg_color(sendBtn_, lv_color_hex(0x2196F3), 0);
    lv_obj_set_style_bg_color(sendBtn_, lv_color_hex(0x42A5F5), LV_STATE_PRESSED);
    lv_obj_set_style_radius(sendBtn_, 16, 0);
    lv_obj_set_style_shadow_width(sendBtn_, 4, 0);
    lv_obj_set_style_shadow_color(sendBtn_, lv_color_hex(0x2196F3), 0);
    lv_obj_set_style_shadow_opa(sendBtn_, LV_OPA_40, 0);
    lv_obj_add_event_cb(sendBtn_, sendBtnCallback, LV_EVENT_CLICKED, this);

    lv_obj_t* btnIcon = lv_label_create(sendBtn_);
    lv_label_set_text(btnIcon, LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_color(btnIcon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(btnIcon);
}

// ============================================================
// 回调函数
// ============================================================

void UIManager::sendBtnCallback(lv_event_t* e) {
    UIManager* ui = static_cast<UIManager*>(lv_event_get_user_data(e));
    if (!ui || !ui->inputArea_) return;
    const char* text = lv_textarea_get_text(ui->inputArea_);
    if (text && strlen(text) > 0 && ui->sendCallback_) {
        std::string msg(text);
        lv_textarea_set_text(ui->inputArea_, "");
        ui->sendCallback_(msg);
    }
}

void UIManager::textareaCallback(lv_event_t* e) {
    UIManager* ui = static_cast<UIManager*>(lv_event_get_user_data(e));
    if (!ui || !ui->inputArea_) return;
    const char* text = lv_textarea_get_text(ui->inputArea_);
    if (text && strlen(text) > 0 && ui->sendCallback_) {
        std::string msg(text);
        lv_textarea_set_text(ui->inputArea_, "");
        ui->sendCallback_(msg);
    }
}

// ============================================================
// 线程安全的数据更新接口
// ============================================================

void UIManager::updateCameraFrame(const cv::Mat& frame) {
    std::lock_guard<std::mutex> lock(frameMutex_);
    frame.copyTo(latestFrame_);
    frameUpdated_ = true;
}

void UIManager::updateEmotion(const std::string& emotion) {
    std::lock_guard<std::mutex> lock(emotionMutex_);
    pendingEmotion_ = emotion;
    emotionUpdated_ = true;
}

void UIManager::addChatMessage(const std::string& text, bool isUser) {
    std::lock_guard<std::mutex> lock(msgMutex_);
    pendingMessages_.push_back({text, isUser});
}

void UIManager::setThinking(bool thinking) {
    std::lock_guard<std::mutex> lock(msgMutex_);
    thinkingState_ = thinking;
    thinkingUpdated_ = true;
}

void UIManager::setSendCallback(std::function<void(const std::string&)> cb) {
    sendCallback_ = cb;
}

// ============================================================
// 气泡数量限制
// ============================================================

void UIManager::trimChatBubbles() {
    if (!chatPanel_ || bubbleCount_ <= MAX_CHAT_BUBBLES) return;

    // 删除最旧的气泡（跳过 thinkingLabel_）
    while (bubbleCount_ > MAX_CHAT_BUBBLES) {
        uint32_t childCount = lv_obj_get_child_count(chatPanel_);
        if (childCount <= 1) break; // 只剩 thinkingLabel_

        lv_obj_t* firstChild = lv_obj_get_child(chatPanel_, 0);
        if (firstChild == thinkingLabel_) {
            if (childCount <= 2) break;
            firstChild = lv_obj_get_child(chatPanel_, 1);
        }
        lv_obj_delete(firstChild);
        bubbleCount_--;
    }
}

// ============================================================
// 内部更新（主线程）
// ============================================================

void UIManager::processUpdates() {
    // 1. 更新摄像头画面
    {
        std::lock_guard<std::mutex> lock(frameMutex_);
        if (frameUpdated_) {
            renderCameraFrame();
            frameUpdated_ = false;
        }
    }

    // 2. 更新情绪胶囊标签
    {
        std::lock_guard<std::mutex> lock(emotionMutex_);
        if (emotionUpdated_ && emotionTag_) {
            // 获取胶囊内的文本标签
            lv_obj_t* tagText = lv_obj_get_child(emotionTag_, 0);
            if (tagText) {
                std::string display = "\xE2\x98\xBA " + pendingEmotion_;  // ☺ + 情绪
                lv_label_set_text(tagText, display.c_str());

                // 根据情绪切换胶囊颜色
                lv_color_t tagColor;
                if (pendingEmotion_.find("\xE5\xBC\x80\xE5\xBF\x83") != std::string::npos ||
                    pendingEmotion_.find("happy") != std::string::npos) {
                    tagColor = lv_color_hex(0x2D6A4F); // 绿
                } else if (pendingEmotion_.find("\xE9\x9A\xBE\xE8\xBF\x87") != std::string::npos ||
                           pendingEmotion_.find("sad") != std::string::npos) {
                    tagColor = lv_color_hex(0x6A2D4F); // 紫红
                } else if (pendingEmotion_.find("\xE7\x94\x9F\xE6\xB0\x94") != std::string::npos ||
                           pendingEmotion_.find("angry") != std::string::npos) {
                    tagColor = lv_color_hex(0x6A2D2D); // 红
                } else {
                    tagColor = lv_color_hex(0x2D4F6A); // 蓝灰（平静）
                }
                lv_obj_set_style_bg_color(emotionTag_, tagColor, 0);
            }
            emotionUpdated_ = false;
        }
    }

    // 3. 更新对话消息
    {
        std::lock_guard<std::mutex> lock(msgMutex_);

        for (const auto& msg : pendingMessages_) {
            if (!chatPanel_) break;

            int maxBubbleW = 230; // right_panel 80% ≈ 230px

            // 创建气泡容器
            lv_obj_t* bubble = lv_obj_create(chatPanel_);
            lv_obj_set_width(bubble, LV_SIZE_CONTENT);
            lv_obj_set_height(bubble, LV_SIZE_CONTENT);
            lv_obj_set_style_max_width(bubble, maxBubbleW, 0);
            lv_obj_set_style_pad_left(bubble, 10, 0);
            lv_obj_set_style_pad_right(bubble, 10, 0);
            lv_obj_set_style_pad_top(bubble, 6, 0);
            lv_obj_set_style_pad_bottom(bubble, 6, 0);
            lv_obj_set_style_border_width(bubble, 0, 0);
            lv_obj_clear_flag(bubble, LV_OBJ_FLAG_SCROLLABLE);

            if (msg.isUser) {
                // 用户消息：靠右，蓝色强调
                lv_obj_set_style_bg_color(bubble, lv_color_hex(0x2196F3), 0);
                lv_obj_set_style_bg_opa(bubble, LV_OPA_90, 0);
                lv_obj_set_style_radius(bubble, 12, 0);
                // 右下角小圆角模拟气泡尾巴
                lv_obj_set_style_radius(bubble, 12, 0);
                lv_obj_set_align(bubble, LV_ALIGN_RIGHT_MID);
            } else {
                // AI 消息：靠左，深灰
                lv_obj_set_style_bg_color(bubble, lv_color_hex(0x23242B), 0);
                lv_obj_set_style_bg_opa(bubble, LV_OPA_80, 0);
                lv_obj_set_style_radius(bubble, 12, 0);
            }

            // 消息文本
            lv_obj_t* label = lv_label_create(bubble);
            lv_label_set_text(label, msg.text.c_str());
            lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
            lv_obj_set_style_max_width(label, maxBubbleW - 20, 0);
            lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_text_font(label, fontCN14_, 0);

            bubbleCount_++;

            // 滚动到底部
            lv_obj_scroll_to_y(chatPanel_, LV_COORD_MAX, LV_ANIM_ON);
        }
        pendingMessages_.clear();

        // 限制气泡数量
        trimChatBubbles();

        // 更新思考状态
        if (thinkingUpdated_ && thinkingLabel_) {
            if (thinkingState_) {
                lv_label_set_text(thinkingLabel_, "思考中...");
                lv_obj_clear_flag(thinkingLabel_, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(thinkingLabel_, LV_OBJ_FLAG_HIDDEN);
            }
            thinkingUpdated_ = false;
        }
    }
}

void UIManager::renderCameraFrame() {
    if (!cameraCanvas_ || latestFrame_.empty()) return;

    // 缩放到 canvas 尺寸 (80x60)
    cv::Mat resized;
    cv::resize(latestFrame_, resized, cv::Size(camW_, camH_));

    // BGR -> BGRA
    cv::Mat bgra;
    cv::cvtColor(resized, bgra, cv::COLOR_BGR2BGRA);

    // 复制到 canvas 缓冲区
    memcpy(canvasBuf_.data(), bgra.data, camW_ * camH_ * 4);

    // 通知 LVGL 更新
    lv_obj_invalidate(cameraCanvas_);
}
