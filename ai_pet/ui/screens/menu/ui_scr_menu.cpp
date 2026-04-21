#include "ui/screens/menu/ui_scr_menu.h"
#include "ui/common/ui_style.h"
#include "ui/common/ui_widgets.h"
#include "ui/managers/ui_manager.h"
#include "ui/screens/home/ui_scr_home.h"
#include "ui/screens/interaction/ui_interaction.h"
#include "ui/screens/memory/ui_memory.h"
#include "ui/screens/persona/ui_persona.h"
#include "ui/screens/settings/ui_settings.h"
#include "ui/screens/tools/ui_tools.h"
#include <cstdint>

namespace {

lv_obj_t* g_screen = nullptr;

static void menu_event_cb(lv_event_t* e) {
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    const auto target = static_cast<pet_ui::manager::ScreenType>(reinterpret_cast<std::uintptr_t>(lv_event_get_user_data(e)));
    switch (target) {
        case pet_ui::manager::ScreenType::Home:
            pet_ui::screen::home::load_screen();
            return;
        case pet_ui::manager::ScreenType::Persona:
            pet_ui::screen::persona::load_screen();
            return;
        case pet_ui::manager::ScreenType::Memory:
            pet_ui::screen::memory::load_screen();
            return;
        case pet_ui::manager::ScreenType::Tools:
            pet_ui::screen::tools::load_screen();
            return;
        case pet_ui::manager::ScreenType::Settings:
            pet_ui::screen::settings::load_screen();
            return;
        case pet_ui::manager::ScreenType::Interaction:
            pet_ui::screen::interaction::load_screen();
            return;
        default:
            return;
    }
}

static void screen_cleanup_cb(lv_event_t*) {
    g_screen = nullptr;
}

static void create_screen() {
    auto* manager = pet_ui::manager::UiManager::getInstance();
    g_screen = lv_obj_create(manager->getContentHost());
    pet_ui::style::style_screen_root(g_screen);
    manager->registerScreen(pet_ui::manager::ScreenType::Menu, &g_screen);
    lv_obj_add_event_cb(g_screen, screen_cleanup_cb, LV_EVENT_DELETE, nullptr);

    lv_obj_t* hero = pet_ui::style::create_card(g_screen, lv_color_hex(0x2B203B), LV_OPA_80);
    pet_ui::style::create_title(hero, "主菜单", manager->getTitleFont());
    pet_ui::style::create_text(hero, "关系与人格、记忆成长、工具、设置、互动模块都从这里进入。", manager->getBodyFont());

    lv_obj_t* grid = lv_obj_create(g_screen);
    lv_obj_set_width(grid, lv_pct(100));
    lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(grid, 0, 0);
    lv_obj_set_style_pad_all(grid, 0, 0);
    lv_obj_set_style_pad_row(grid, 8, 0);
    lv_obj_set_style_pad_column(grid, 8, 0);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);

    struct MenuItem { const char* title; const char* subtitle; pet_ui::manager::ScreenType target; };
    const MenuItem items[] = {
        {"关系与人格", "角色、人设、关系状态", pet_ui::manager::ScreenType::Persona},
        {"记忆与成长", "历史、重要记忆、时间线", pet_ui::manager::ScreenType::Memory},
        {"实用工具", "翻译、天气、备忘", pet_ui::manager::ScreenType::Tools},
        {"系统设置", "摄像头、性能、主题", pet_ui::manager::ScreenType::Settings},
        {"扩展互动", "主动对话、打扰模式", pet_ui::manager::ScreenType::Interaction},
        {"返回陪伴页", "回到主界面继续聊天", pet_ui::manager::ScreenType::Home},
    };

    for (const auto& item : items) {
        lv_obj_t* button = pet_ui::widgets::create_menu_button(grid, item.title, item.subtitle, manager->getBodyFont());
        lv_obj_add_event_cb(button, menu_event_cb, LV_EVENT_CLICKED, reinterpret_cast<void*>(static_cast<std::uintptr_t>(item.target)));
    }
}

}  // namespace

namespace pet_ui::screen::menu {

void load_screen() {
    auto* manager = pet_ui::manager::UiManager::getInstance();
    if (!g_screen) {
        create_screen();
    }
    manager->resetKeypadGroup();
    uint32_t count = lv_obj_get_child_count(lv_obj_get_child(g_screen, 1));
    lv_obj_t* grid = lv_obj_get_child(g_screen, 1);
    for (uint32_t i = 0; i < count; ++i) {
        manager->addObjToGroup(lv_obj_get_child(grid, i));
    }
    if (count > 0) {
        lv_group_focus_obj(lv_obj_get_child(grid, 0));
    }
    manager->loadContentScreen(pet_ui::manager::ScreenType::Menu, g_screen);
}

}  // namespace pet_ui::screen::menu
