#include "ui/screens/persona/ui_persona.h"
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
    manager->registerScreen(pet_ui::manager::ScreenType::Persona, &g_screen);
    lv_obj_add_event_cb(g_screen, screen_cleanup_cb, LV_EVENT_DELETE, nullptr);

    lv_obj_t* header = pet_ui::style::create_card(g_screen, lv_color_hex(0x352B3F), LV_OPA_80);
    pet_ui::style::create_title(header, "关系与人格", manager->getTitleFont());
    pet_ui::style::create_text(header, "角色：默认\n性格：克制 / 疏离 / 吃醋\n当前状态：有点动摇", manager->getBodyFont());

    lv_obj_t* relation = pet_ui::style::create_card(g_screen);
    pet_ui::style::create_title(relation, "关系状态", manager->getTitleFont());
    pet_ui::style::create_text(relation, "好感度：Lv2\n关系：若即若离\n她对你：还在观察", manager->getBodyFont());

    lv_obj_t* trend = pet_ui::style::create_card(g_screen, lv_color_hex(0x22263A), LV_OPA_80);
    pet_ui::style::create_title(trend, "人格变化趋势", manager->getTitleFont());
    pet_ui::style::create_text(trend, "最近变化：\n- 更温和\n- 更愿意回应", manager->getBodyFont());

    g_back = pet_ui::widgets::create_back_button(g_screen, "返回菜单", manager->getBodyFont());
    lv_obj_add_event_cb(g_back, screen_event_cb, LV_EVENT_CLICKED, nullptr);
}

}  // namespace

namespace pet_ui::screen::persona {

void load_screen() {
    auto* manager = pet_ui::manager::UiManager::getInstance();
    if (!g_screen) {
        create_screen();
    }
    manager->resetKeypadGroup();
    manager->addObjToGroup(g_back);
    lv_group_focus_obj(g_back);
    manager->loadContentScreen(pet_ui::manager::ScreenType::Persona, g_screen);
}

}  // namespace pet_ui::screen::persona
