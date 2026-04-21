#pragma once

#include "app/app_event_bus.h"
#include "ui/managers/ui_screen_lifecycle.h"
#include <cstdint>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

namespace pet_ui::controller {

struct ChatMessage {
    std::string text;
    bool isUser;
    bool isProactive;
    std::string proactiveType;
};

class UiController {
public:
    using ScreenType = pet_ui::manager::UiScreenLifecycleManager::ScreenType;

    static UiController* getInstance();

    void init(AppEventBus* eventBus);
    void processPending();

    void updateCameraFrame(const cv::Mat& frame);
    void showNotification(const std::string& text);
    void switchScreen(ScreenType screenType);

private:
    void bindEventBus();
    void loadScreen(ScreenType screenType);

    AppEventBus* eventBus_ = nullptr;
    bool bound_ = false;

    std::mutex frameMutex_;
    cv::Mat latestFrame_;
    bool frameUpdated_ = false;
    uint32_t lastFrameCommitMs_ = 0;
    bool petPerfModeEnabled_ = false;

    std::mutex notificationMutex_;
    std::string pendingNotification_;
    bool notificationUpdated_ = false;

    std::mutex screenMutex_;
    ScreenType pendingScreen_ = ScreenType::Unknown;
    bool screenUpdated_ = false;
};

}  // namespace pet_ui::controller
