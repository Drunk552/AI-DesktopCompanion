#include "ui/screens/tools/ui_tools.h"
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
    manager->registerScreen(pet_ui::manager::ScreenType::Tools, &g_screen);
    lv_obj_add_event_cb(g_screen, screen_cleanup_cb, LV_EVENT_DELETE, nullptr);

    lv_obj_t* translate = pet_ui::style::create_card(g_screen, lv_color_hex(0x23404A), LV_OPA_80);
    pet_ui::style::create_title(translate, "翻译", manager->getTitleFont());
    pet_ui::style::create_text(translate, "输入文本 -> 翻译结果\nAI 会用陪伴式语气表达结果。", manager->getBodyFont());

    lv_obj_t* weather = pet_ui::style::create_card(g_screen);
    pet_ui::style::create_title(weather, "日期 / 天气", manager->getTitleFont());
    pet_ui::style::create_text(weather, "今天：周三\n天气：小雨\nAI：……你不是很喜欢这种天气吗？", manager->getBodyFont());

    lv_obj_t* memo = pet_ui::style::create_card(g_screen, lv_color_hex(0x403D2B), LV_OPA_80);
    pet_ui::style::create_title(memo, "备忘录", manager->getTitleFont());
    pet_ui::style::create_text(memo, "- 写代码\n- 吃饭\nAI 提醒：你又忘了。", manager->getBodyFont());

    g_back = pet_ui::widgets::create_back_button(g_screen, "返回菜单", manager->getBodyFont());
    lv_obj_add_event_cb(g_back, screen_event_cb, LV_EVENT_CLICKED, nullptr);
}

}  // namespace

namespace pet_ui::screen::tools {

void load_screen() {
    auto* manager = pet_ui::manager::UiManager::getInstance();
    if (!g_screen) {
        create_screen();
    }
    manager->resetKeypadGroup();
    manager->addObjToGroup(g_back);
    lv_group_focus_obj(g_back);
    manager->loadContentScreen(pet_ui::manager::ScreenType::Tools, g_screen);
}

}  // namespace pet_ui::screen::tools
