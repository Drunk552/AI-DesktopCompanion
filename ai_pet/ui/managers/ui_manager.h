#pragma once

#include "app/app_event_bus.h"
#include "lvgl.h"
#include "ui/managers/ui_screen_lifecycle.h"

namespace pet_ui::manager {

using ScreenType = UiScreenLifecycleManager::ScreenType;

class UiManager {
public:
    static UiManager* getInstance();

    void init(int width, int height, lv_group_t* keypadGroup, lv_font_t* bodyFont, lv_font_t* titleFont, AppEventBus* eventBus);
    void registerScreen(ScreenType type, lv_obj_t** screenPtrRef, UiScreenLifecycleManager::ScreenCategory category = UiScreenLifecycleManager::ScreenCategory::ContentScreen);
    void loadContentScreen(ScreenType type, lv_obj_t* screen);
    void destroyAllScreensExcept(lv_obj_t* keep);
    void resetKeypadGroup();
    void addObjToGroup(lv_obj_t* obj);

    lv_group_t* getKeypadGroup() const { return keypadGroup_; }
    lv_obj_t* getRootScreen() const { return rootScreen_; }
    lv_obj_t* getLeftPanel() const { return leftPanel_; }
    lv_obj_t* getStatusHost() const { return statusHost_; }
    lv_obj_t* getContentHost() const { return contentHost_; }
    lv_font_t* getBodyFont() const { return bodyFont_; }
    lv_font_t* getTitleFont() const { return titleFont_; }
    AppEventBus* getEventBus() const { return eventBus_; }

private:
    UiManager() = default;

    int width_ = 480;
    int height_ = 320;
    lv_group_t* keypadGroup_ = nullptr;
    lv_font_t* bodyFont_ = nullptr;
    lv_font_t* titleFont_ = nullptr;
    AppEventBus* eventBus_ = nullptr;

    lv_obj_t* rootScreen_ = nullptr;
    lv_obj_t* rootRow_ = nullptr;
    lv_obj_t* leftPanel_ = nullptr;
    lv_obj_t* rightPanel_ = nullptr;
    lv_obj_t* statusHost_ = nullptr;
    lv_obj_t* contentHost_ = nullptr;
};

}  // namespace pet_ui::manager
