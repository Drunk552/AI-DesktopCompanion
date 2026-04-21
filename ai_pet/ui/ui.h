/**
 * @file ui/ui.h
 * @brief UI 模块兼容入口
 */

#pragma once

#include "app/app_event_bus.h"
#include "ui/managers/ui_screen_lifecycle.h"
#include <opencv2/opencv.hpp>
#include <string>

class UIManager {
public:
    using ScreenType = pet_ui::manager::UiScreenLifecycleManager::ScreenType;

    UIManager();
    ~UIManager();

    void setEventBus(AppEventBus* eventBus) { eventBus_ = eventBus; }
    bool init(int width = 480, int height = 320);
    bool tick();
    void shutdown();

    void updateCameraFrame(const cv::Mat& frame);
    void showNotification(const std::string& text);
    void switchScreen(ScreenType screenType);

    bool isReady() const { return initialized_; }
    AppEventBus& eventBus() { return *eventBus_; }

private:
    bool initialized_ = false;
    AppEventBus* eventBus_ = nullptr;
};
