#include "ui/managers/ui_manager.h"

namespace pet_ui::manager {

UiManager* UiManager::getInstance() {
    static UiManager instance;
    return &instance;
}

void UiManager::init(int width, int height, lv_group_t* keypadGroup, lv_font_t* bodyFont, lv_font_t* titleFont, AppEventBus* eventBus) {
    width_ = width;
    height_ = height;
    keypadGroup_ = keypadGroup;
    bodyFont_ = bodyFont;
    titleFont_ = titleFont;
    eventBus_ = eventBus;

    rootScreen_ = lv_obj_create(nullptr);
    lv_obj_set_size(rootScreen_, width_, height_);
    lv_obj_set_style_bg_color(rootScreen_, lv_color_hex(0x10131A), 0);
    lv_obj_set_style_border_width(rootScreen_, 0, 0);
    lv_obj_set_style_pad_all(rootScreen_, 0, 0);

    rootRow_ = lv_obj_create(rootScreen_);
    lv_obj_set_size(rootRow_, width_, height_);
    lv_obj_set_style_bg_opa(rootRow_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(rootRow_, 0, 0);
    lv_obj_set_style_pad_all(rootRow_, 0, 0);
    lv_obj_set_style_pad_column(rootRow_, 0, 0);
    lv_obj_set_flex_flow(rootRow_, LV_FLEX_FLOW_ROW);
    lv_obj_clear_flag(rootRow_, LV_OBJ_FLAG_SCROLLABLE);

    leftPanel_ = lv_obj_create(rootRow_);
    lv_obj_set_size(leftPanel_, width_ * 2 / 5, height_);
    lv_obj_set_style_border_width(leftPanel_, 0, 0);
    lv_obj_set_style_pad_all(leftPanel_, 0, 0);
    lv_obj_set_style_radius(leftPanel_, 0, 0);
    lv_obj_clear_flag(leftPanel_, LV_OBJ_FLAG_SCROLLABLE);

    rightPanel_ = lv_obj_create(rootRow_);
    lv_obj_set_size(rightPanel_, width_ - (width_ * 2 / 5), height_);
    lv_obj_set_style_border_width(rightPanel_, 0, 0);
    lv_obj_set_style_pad_all(rightPanel_, 0, 0);
    lv_obj_set_style_pad_row(rightPanel_, 0, 0);
    lv_obj_set_style_radius(rightPanel_, 0, 0);
    lv_obj_set_flex_flow(rightPanel_, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(rightPanel_, LV_OBJ_FLAG_SCROLLABLE);

    statusHost_ = lv_obj_create(rightPanel_);
    lv_obj_set_size(statusHost_, width_ - (width_ * 2 / 5), 48);
    lv_obj_set_style_bg_opa(statusHost_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(statusHost_, 0, 0);
    lv_obj_set_style_pad_all(statusHost_, 0, 0);
    lv_obj_clear_flag(statusHost_, LV_OBJ_FLAG_SCROLLABLE);

    contentHost_ = lv_obj_create(rightPanel_);
    lv_obj_set_width(contentHost_, width_ - (width_ * 2 / 5));
    lv_obj_set_flex_grow(contentHost_, 1);
    lv_obj_set_style_bg_opa(contentHost_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(contentHost_, 0, 0);
    lv_obj_set_style_pad_all(contentHost_, 0, 0);
    lv_obj_clear_flag(contentHost_, LV_OBJ_FLAG_SCROLLABLE);

    lv_screen_load(rootScreen_);
}

void UiManager::registerScreen(ScreenType type, lv_obj_t** screenPtrRef, UiScreenLifecycleManager::ScreenCategory category) {
    UiScreenLifecycleManager::getInstance()->registerScreen(type, category, screenPtrRef);
}

void UiManager::loadContentScreen(ScreenType, lv_obj_t* screen) {
    if (!screen) {
        return;
    }
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_HIDDEN);
    destroyAllScreensExcept(screen);
}

void UiManager::destroyAllScreensExcept(lv_obj_t* keep) {
    UiScreenLifecycleManager::getInstance()->destroyAllScreensExcept(keep);
}

void UiManager::resetKeypadGroup() {
    if (keypadGroup_) {
        lv_group_remove_all_objs(keypadGroup_);
    }
}

void UiManager::addObjToGroup(lv_obj_t* obj) {
    if (keypadGroup_ && obj) {
        lv_group_add_obj(keypadGroup_, obj);
    }
}

}  // namespace pet_ui::manager
