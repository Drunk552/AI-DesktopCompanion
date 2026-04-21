#include "ui/screens/memory/ui_memory.h"
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
    manager->registerScreen(pet_ui::manager::ScreenType::Memory, &g_screen);
    lv_obj_add_event_cb(g_screen, screen_cleanup_cb, LV_EVENT_DELETE, nullptr);

    lv_obj_t* review = pet_ui::style::create_card(g_screen, lv_color_hex(0x253046), LV_OPA_80);
    pet_ui::style::create_title(review, "记忆回顾", manager->getTitleFont());
    pet_ui::style::create_text(review, "- 最近一次对话\n- 聊天上下文\n- 情绪轨迹", manager->getBodyFont());

    lv_obj_t* key = pet_ui::style::create_card(g_screen);
    pet_ui::style::create_title(key, "重要记忆", manager->getTitleFont());
    pet_ui::style::create_text(key, "- 你说你很累\n- 你喜欢下雨\n- 你昨天没理她", manager->getBodyFont());

    lv_obj_t* timeline = pet_ui::style::create_card(g_screen, lv_color_hex(0x2E3440), LV_OPA_80);
    pet_ui::style::create_title(timeline, "关系时间线", manager->getTitleFont());
    pet_ui::style::create_text(timeline, "Day1：初识（冷淡）\nDay3：开始回应\nDay7：出现情绪波动", manager->getBodyFont());

    g_back = pet_ui::widgets::create_back_button(g_screen, "返回菜单", manager->getBodyFont());
    lv_obj_add_event_cb(g_back, screen_event_cb, LV_EVENT_CLICKED, nullptr);
}

}  // namespace

namespace pet_ui::screen::memory {

void load_screen() {
    auto* manager = pet_ui::manager::UiManager::getInstance();
    if (!g_screen) {
        create_screen();
    }
    manager->resetKeypadGroup();
    manager->addObjToGroup(g_back);
    lv_group_focus_obj(g_back);
    manager->loadContentScreen(pet_ui::manager::ScreenType::Memory, g_screen);
}

}  // namespace pet_ui::screen::memory
