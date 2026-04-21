#pragma once

#include "app/app_event_bus.h"
#include "ui/managers/ui_screen_lifecycle.h"

namespace pet_ui::navigation {

using ScreenType = manager::UiScreenLifecycleManager::ScreenType;

inline constexpr const char* kScreenSwitchEvent = "ui.screen.switch";

struct ScreenSwitchRequest {
    ScreenType target = ScreenType::Unknown;
};

inline void publishScreenSwitch(AppEventBus& eventBus, ScreenType target) {
    eventBus.emitTyped(kScreenSwitchEvent, ScreenSwitchRequest{target});
}

}  // namespace pet_ui::navigation
