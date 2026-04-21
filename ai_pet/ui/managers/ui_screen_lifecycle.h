#pragma once

#include "lvgl.h"
#include <string>
#include <vector>

namespace pet_ui::manager {

class UiScreenLifecycleManager {
public:
    enum class ScreenCategory {
        ContentScreen,
        OverlayLayer,
    };

    enum class ScreenType {
        Home,
        Menu,
        Persona,
        Memory,
        Tools,
        Settings,
        Interaction,
        PetOverlay,
        StatusOverlay,
        NotificationOverlay,
        Unknown,
    };

    static UiScreenLifecycleManager* getInstance();

    void registerScreen(ScreenType type, ScreenCategory category, lv_obj_t** screenPtrRef);
    void destroyAllScreensExcept(lv_obj_t* keepScreen);

private:
    UiScreenLifecycleManager() = default;

    struct ManagedScreen {
        ScreenType type;
        ScreenCategory category;
        lv_obj_t** ptrRef;
    };

    static void async_screen_cleanup_cb(lv_timer_t* timer);
    static const char* screenTypeToString(ScreenType type);
    static const char* screenCategoryToString(ScreenCategory category);

    lv_obj_t* keepScreen_ = nullptr;
    std::vector<ManagedScreen> managedScreens_;
};

}  // namespace pet_ui::manager
