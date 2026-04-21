#include "ui/screens/settings/ui_settings.h"
#include "ui/common/ui_style.h"
#include "ui/common/ui_widgets.h"
#include "ui/managers/ui_manager.h"
#include "ui/screens/menu/ui_scr_menu.h"

namespace {

lv_obj_t* g_screen = nullptr;
lv_obj_t* g_back = nullptr;

static void screen_event_cb(lv_event_t* e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED && lv_event_get_target(e) == g_back) {
        pet_ui::screen::menu::load_screen();
    }
}

static void screen_cleanup_cb(lv_event_t*) {
    g_screen = nullptr;
    g_back = nullptr;
}

static void create_screen() {
    auto* manager = pet_ui::manager::UiManager::getInstance();
    g_screen = lv_obj_create(manager->getContentHost());
    pet_ui::style::style_screen_root(g_screen);
    manager->registerScreen(pet_ui::manager::ScreenType::Settings, &g_screen);
    lv_obj_add_event_cb(g_screen, screen_cleanup_cb, LV_EVENT_DELETE, nullptr);

    lv_obj_t* system = pet_ui::style::create_card(g_screen, lv_color_hex(0x2B3A2F), LV_OPA_80);
    pet_ui::style::create_title(system, "系统控制", manager->getTitleFont());
    pet_ui::style::create_text(system, "- 摄像头开关\n- 表情识别开关\n- 模型选择（Gemma 等）", manager->getBodyFont());

    lv_obj_t* perf = pet_ui::style::create_card(g_screen);
    pet_ui::style::create_title(perf, "性能设置", manager->getTitleFont());
    pet_ui::style::create_text(perf, "- 动画开关\n- 帧率限制\n- 低功耗模式", manager->getBodyFont());

    lv_obj_t* theme = pet_ui::style::create_card(g_screen, lv_color_hex(0x2B303B), LV_OPA_80);
    pet_ui::style::create_title(theme, "UI 设置", manager->getTitleFont());
    pet_ui::style::create_text(theme, "- 主题（深色 / 冷色）\n- 字体大小", manager->getBodyFont());

    g_back = pet_ui::widgets::create_back_button(g_screen, "返回菜单", manager->getBodyFont());
    lv_obj_add_event_cb(g_back, screen_event_cb, LV_EVENT_CLICKED, nullptr);
}

}  // namespace

namespace pet_ui::screen::settings {

void load_screen() {
    auto* manager = pet_ui::manager::UiManager::getInstance();
    if (!g_screen) {
        create_screen();
    }
    manager->resetKeypadGroup();
    manager->addObjToGroup(g_back);
    lv_group_focus_obj(g_back);
    manager->loadContentScreen(pet_ui::manager::ScreenType::Settings, g_screen);
}

}  // namespace pet_ui::screen::settings
