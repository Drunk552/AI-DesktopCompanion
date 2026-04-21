#include "ui/ui.h"
#include "ui/controller/ui_controller.h"
#include "ui/ui_app.h"

UIManager::UIManager() = default;

UIManager::~UIManager() {
    shutdown();
}

bool UIManager::init(int width, int height) {
    if (initialized_) {
        return true;
    }
    initialized_ = pet_ui::app::init(width, height, eventBus_);
    return initialized_;
}

bool UIManager::tick() {
    if (!initialized_) {
        return false;
    }
    return pet_ui::app::tick();
}

void UIManager::shutdown() {
    if (!initialized_) {
        return;
    }
    pet_ui::app::shutdown();
    initialized_ = false;
}

void UIManager::updateCameraFrame(const cv::Mat& frame) {
    pet_ui::controller::UiController::getInstance()->updateCameraFrame(frame);
}

void UIManager::showNotification(const std::string& text) {
    pet_ui::controller::UiController::getInstance()->showNotification(text);
}

void UIManager::switchScreen(ScreenType screenType) {
    pet_ui::controller::UiController::getInstance()->switchScreen(screenType);
}
