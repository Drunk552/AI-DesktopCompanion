#include "ui/controller/ui_controller.h"
#include "action/action_events.h"
#include "app/settings_events.h"
#include "brain/brain_events.h"
#include "brain/brain_types.h"
#include "ui/controller/ui_navigation.h"
#include "ui/overlay/ui_notification.h"
#include "ui/overlay/ui_pet_layer.h"
#include "ui/overlay/ui_status_bar.h"
#include "ui/screens/home/ui_scr_home.h"
#include "ui/screens/interaction/ui_interaction.h"
#include "ui/screens/memory/ui_memory.h"
#include "ui/screens/menu/ui_scr_menu.h"
#include "ui/screens/persona/ui_persona.h"
#include "ui/screens/settings/ui_settings.h"
#include "ui/screens/tools/ui_tools.h"
#include <SDL2/SDL.h>

namespace pet_ui::controller {

UiController* UiController::getInstance() {
    static UiController instance;
    return &instance;
}

void UiController::init(AppEventBus* eventBus) {
    eventBus_ = eventBus;
    bindEventBus();
}

void UiController::bindEventBus() {
    if (!eventBus_ || bound_) {
        return;
    }

    eventBus_->subscribe(brain::events::kUserInputText, [this](const std::string& text) {
        switchScreen(ScreenType::Home);
    });
    eventBus_->subscribeTyped<action::events::NotificationEvent>(action::events::kNotificationShow, [this](const action::events::NotificationEvent& event) { showNotification(event.text); });
    eventBus_->subscribeTyped<action::events::ProactiveBehaviorEvent>(action::events::kBehaviorProactiveCare, [this](const action::events::ProactiveBehaviorEvent&) { switchScreen(ScreenType::Home); });
    eventBus_->subscribeTyped<action::events::ProactiveBehaviorEvent>(action::events::kBehaviorProactiveCheckIn, [this](const action::events::ProactiveBehaviorEvent&) { switchScreen(ScreenType::Home); });
    eventBus_->subscribeTyped<action::events::ProactiveBehaviorEvent>(action::events::kBehaviorProactiveTease, [this](const action::events::ProactiveBehaviorEvent&) { switchScreen(ScreenType::Home); });
    eventBus_->subscribeTyped<action::events::ProactiveBehaviorEvent>(action::events::kBehaviorProactiveReminder, [this](const action::events::ProactiveBehaviorEvent&) { switchScreen(ScreenType::Home); });
    eventBus_->subscribeTyped<pet_ui::navigation::ScreenSwitchRequest>(pet_ui::navigation::kScreenSwitchEvent, [this](const pet_ui::navigation::ScreenSwitchRequest& request) {
        switchScreen(request.target);
    });
    bound_ = true;
}

void UiController::updateCameraFrame(const cv::Mat& frame) {
    std::lock_guard<std::mutex> lock(frameMutex_);
    frame.copyTo(latestFrame_);
    frameUpdated_ = true;
}

void UiController::showNotification(const std::string& text) {
    std::lock_guard<std::mutex> lock(notificationMutex_);
    pendingNotification_ = text;
    notificationUpdated_ = true;
}

void UiController::switchScreen(ScreenType screenType) {
    std::lock_guard<std::mutex> lock(screenMutex_);
    pendingScreen_ = screenType;
    screenUpdated_ = true;
}

void UiController::loadScreen(ScreenType screenType) {
    switch (screenType) {
        case ScreenType::Home:
            pet_ui::screen::home::load_screen();
            return;
        case ScreenType::Menu:
            pet_ui::screen::menu::load_screen();
            return;
        case ScreenType::Persona:
            pet_ui::screen::persona::load_screen();
            return;
        case ScreenType::Memory:
            pet_ui::screen::memory::load_screen();
            return;
        case ScreenType::Tools:
            pet_ui::screen::tools::load_screen();
            return;
        case ScreenType::Settings:
            pet_ui::screen::settings::load_screen();
            return;
        case ScreenType::Interaction:
            pet_ui::screen::interaction::load_screen();
            return;
        default:
            return;
    }
}

void UiController::processPending() {
    const uint32_t now = SDL_GetTicks();
    const bool scrollActive = pet_ui::screen::home::is_scroll_active();
    if (petPerfModeEnabled_ != scrollActive) {
        pet_ui::overlay::pet_layer::set_performance_mode(scrollActive);
        petPerfModeEnabled_ = scrollActive;
    }

    {
        std::lock_guard<std::mutex> lock(screenMutex_);
        if (screenUpdated_) {
            loadScreen(pendingScreen_);
            screenUpdated_ = false;
        }
    }

    {
        std::lock_guard<std::mutex> lock(frameMutex_);
        if (frameUpdated_) {
            const uint32_t min_interval = scrollActive ? 180 : 80;
            if (lastFrameCommitMs_ == 0 || now - lastFrameCommitMs_ >= min_interval) {
                pet_ui::overlay::pet_layer::update_camera_frame(latestFrame_);
                lastFrameCommitMs_ = now;
                frameUpdated_ = false;
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(notificationMutex_);
        if (notificationUpdated_) {
            pet_ui::overlay::notification::show(pendingNotification_);
            notificationUpdated_ = false;
        }
    }
}

}  // namespace pet_ui::controller
