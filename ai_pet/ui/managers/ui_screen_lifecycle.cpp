#include "ui/managers/ui_screen_lifecycle.h"
#include "shared/logger/logger.h"
#include <cstdint>
#include <iomanip>
#include <sstream>

namespace {

std::string ptr_to_string(const void* ptr) {
    std::ostringstream oss;
    oss << "0x" << std::hex << reinterpret_cast<std::uintptr_t>(ptr);
    return oss.str();
}

}

namespace pet_ui::manager {

UiScreenLifecycleManager* UiScreenLifecycleManager::getInstance() {
    static UiScreenLifecycleManager instance;
    return &instance;
}

void UiScreenLifecycleManager::registerScreen(ScreenType type, ScreenCategory category, lv_obj_t** screenPtrRef) {
    if (!screenPtrRef) {
        return;
    }

    for (auto& managed : managedScreens_) {
        if (managed.ptrRef == screenPtrRef) {
            managed.type = type;
            managed.category = category;
            LOGI("UI.Lifecycle", std::string("注册页面(已存在): ") + screenTypeToString(type) + " [" + screenCategoryToString(category) + "] @" + ptr_to_string(*screenPtrRef));
            return;
        }
    }

    managedScreens_.push_back({type, category, screenPtrRef});
    LOGI("UI.Lifecycle", std::string("注册页面: ") + screenTypeToString(type) + " [" + screenCategoryToString(category) + "] @" + ptr_to_string(*screenPtrRef));
}

void UiScreenLifecycleManager::destroyAllScreensExcept(lv_obj_t* keepScreen) {
    keepScreen_ = keepScreen;
    for (const auto& managed : managedScreens_) {
        if (managed.ptrRef && *managed.ptrRef == keepScreen_) {
            LOGI("UI.Lifecycle", std::string("保留页面: ") + screenTypeToString(managed.type) + " [" + screenCategoryToString(managed.category) + "] @" + ptr_to_string(*managed.ptrRef));
            break;
        }
    }
    lv_timer_t* timer = lv_timer_create(async_screen_cleanup_cb, 10, this);
    lv_timer_set_repeat_count(timer, 1);
}

void UiScreenLifecycleManager::async_screen_cleanup_cb(lv_timer_t* timer) {
    UiScreenLifecycleManager* manager = static_cast<UiScreenLifecycleManager*>(lv_timer_get_user_data(timer));
    if (!manager) {
        return;
    }

    for (auto& managed : manager->managedScreens_) {
        if (managed.category != ScreenCategory::ContentScreen) {
            continue;
        }

        if (managed.ptrRef && *managed.ptrRef && *managed.ptrRef != manager->keepScreen_) {
            LOGI("UI.Lifecycle", std::string("异步销毁页面: ") + screenTypeToString(managed.type) + " [" + screenCategoryToString(managed.category) + "] @" + ptr_to_string(*managed.ptrRef));
            lv_obj_delete(*managed.ptrRef);
            *managed.ptrRef = nullptr;
        }
    }
}

const char* UiScreenLifecycleManager::screenTypeToString(ScreenType type) {
    switch (type) {
        case ScreenType::Home: return "Home";
        case ScreenType::Menu: return "Menu";
        case ScreenType::Persona: return "Persona";
        case ScreenType::Memory: return "Memory";
        case ScreenType::Tools: return "Tools";
        case ScreenType::Settings: return "Settings";
        case ScreenType::Interaction: return "Interaction";
        case ScreenType::PetOverlay: return "PetOverlay";
        case ScreenType::StatusOverlay: return "StatusOverlay";
        case ScreenType::NotificationOverlay: return "NotificationOverlay";
        case ScreenType::Unknown: return "Unknown";
    }
    return "Unknown";
}

const char* UiScreenLifecycleManager::screenCategoryToString(ScreenCategory category) {
    switch (category) {
        case ScreenCategory::ContentScreen: return "ContentScreen";
        case ScreenCategory::OverlayLayer: return "OverlayLayer";
    }
    return "ContentScreen";
}

}  // namespace pet_ui::manager
