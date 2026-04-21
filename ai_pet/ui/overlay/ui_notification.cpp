#include "ui/overlay/ui_notification.h"
#include "ui/managers/ui_manager.h"

namespace {

lv_obj_t* g_root = nullptr;
lv_obj_t* g_label = nullptr;
uint32_t g_show_tick = 0;

}  // namespace

namespace pet_ui::overlay::notification {

void create() {
    auto* manager = pet_ui::manager::UiManager::getInstance();
    g_root = lv_obj_create(manager->getRootScreen());
    lv_obj_set_size(g_root, 220, LV_SIZE_CONTENT);
    lv_obj_align(g_root, LV_ALIGN_TOP_RIGHT, -18, 18);
    lv_obj_set_style_bg_color(g_root, lv_color_hex(0x2A2438), 0);
    lv_obj_set_style_bg_opa(g_root, LV_OPA_90, 0);
    lv_obj_set_style_border_width(g_root, 0, 0);
    lv_obj_set_style_radius(g_root, 18, 0);
    lv_obj_set_style_pad_all(g_root, 12, 0);
    lv_obj_add_flag(g_root, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(g_root, LV_OBJ_FLAG_SCROLLABLE);
    manager->registerScreen(
        pet_ui::manager::ScreenType::NotificationOverlay,
        &g_root,
        pet_ui::manager::UiScreenLifecycleManager::ScreenCategory::OverlayLayer
    );

    g_label = lv_label_create(g_root);
    lv_label_set_long_mode(g_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(g_label, lv_pct(100));
    lv_obj_set_style_text_color(g_label, lv_color_hex(0xF9FAFB), 0);
    if (manager->getBodyFont()) {
        lv_obj_set_style_text_font(g_label, manager->getBodyFont(), 0);
    }
}

void show(const std::string& text) {
    if (!g_root || !g_label) {
        return;
    }
    lv_label_set_text(g_label, text.c_str());
    lv_obj_clear_flag(g_root, LV_OBJ_FLAG_HIDDEN);
    g_show_tick = lv_tick_get();
}

void tick() {
    if (g_root && g_show_tick != 0 && lv_tick_elaps(g_show_tick) > 2200) {
        lv_obj_add_flag(g_root, LV_OBJ_FLAG_HIDDEN);
        g_show_tick = 0;
    }
}

}  // namespace pet_ui::overlay::notification
