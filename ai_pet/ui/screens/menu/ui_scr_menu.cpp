#include "ui/screens/menu/ui_scr_menu.h"
#include "ui/managers/ui_manager.h"
#include "ui/screens/interaction/ui_interaction.h"
#include "ui/screens/memory/ui_memory.h"
#include "ui/screens/persona/ui_persona.h"
#include "ui/screens/settings/ui_settings.h"
#include "ui/screens/tools/ui_tools.h"
#include <cstdio>
#include <cstdint>
#include <ctime>
#include <string>
#include <vector>

namespace {

lv_obj_t* g_screen = nullptr;
lv_obj_t* g_top_left = nullptr;
lv_obj_t* g_top_right = nullptr;
std::vector<lv_obj_t*> g_menu_buttons;

struct MenuItem {
    const char* icon;
    const char* english;
    const char* chinese;
    uint32_t colorHex;
    pet_ui::manager::ScreenType target;
};

constexpr MenuItem kMenuItems[] = {
    {LV_SYMBOL_EYE_OPEN, "Persona", "人格系统模块", 0xD96C18, pet_ui::manager::ScreenType::Persona},
    {LV_SYMBOL_FILE, "Memory", "记忆系统模块", 0x2F6FF3, pet_ui::manager::ScreenType::Memory},
    {LV_SYMBOL_DIRECTORY, "Tools", "工具模块", 0x2B74F5, pet_ui::manager::ScreenType::Tools},
    {LV_SYMBOL_SETTINGS, "System", "系统设置模块", 0x3A7AF7, pet_ui::manager::ScreenType::Settings},
    {LV_SYMBOL_LOOP, "Interaction", "扩展互动模块", 0x245FE0, pet_ui::manager::ScreenType::Interaction},
};

std::string current_menu_date_label() {
    std::time_t now = std::time(nullptr);
    std::tm local_tm{};
#if defined(_WIN32)
    localtime_s(&local_tm, &now);
#else
    localtime_r(&now, &local_tm);
#endif

    char buffer[32];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "%04d-%02d-%02d",
        local_tm.tm_year + 1900,
        local_tm.tm_mon + 1,
        local_tm.tm_mday
    );
    return buffer;
}

std::string current_menu_time_label() {
    std::time_t now = std::time(nullptr);
    std::tm local_tm{};
#if defined(_WIN32)
    localtime_s(&local_tm, &now);
#else
    localtime_r(&now, &local_tm);
#endif

    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d", local_tm.tm_hour, local_tm.tm_min);
    return buffer;
}

void refresh_top_bar_clock() {
    if (!g_top_left || !g_top_right) {
        return;
    }

    const std::string dateText = current_menu_date_label();
    const std::string timeText = current_menu_time_label();
    lv_label_set_text(g_top_left, dateText.c_str());
    lv_label_set_text(g_top_right, timeText.c_str());
}

static void open_target(pet_ui::manager::ScreenType target) {
    switch (target) {
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

static void menu_event_cb(lv_event_t* e) {
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    const auto target = static_cast<pet_ui::manager::ScreenType>(
        reinterpret_cast<std::uintptr_t>(lv_event_get_user_data(e))
    );
    open_target(target);
}

static void screen_cleanup_cb(lv_event_t*) {
    g_screen = nullptr;
    g_top_left = nullptr;
    g_top_right = nullptr;
    g_menu_buttons.clear();
}

static lv_obj_t* create_menu_tile(
    lv_obj_t* parent,
    const MenuItem& item,
    const lv_font_t* titleFont,
    const lv_font_t* bodyFont
) {
    lv_obj_t* button = lv_button_create(parent);
    const lv_color_t buttonColor = lv_color_hex(item.colorHex);
    lv_obj_set_size(button, 128, 70);
    lv_obj_set_style_bg_color(button, buttonColor, 0);
    lv_obj_set_style_bg_grad_color(button, lv_color_hex(0x2D89FF), 0);
    lv_obj_set_style_bg_grad_dir(button, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_border_color(button, lv_color_hex(0x93C5FD), 0);
    lv_obj_set_style_radius(button, 10, 0);
    lv_obj_set_style_shadow_width(button, 6, 0);
    lv_obj_set_style_shadow_opa(button, LV_OPA_10, 0);
    lv_obj_set_style_shadow_color(button, lv_color_hex(0x123A8C), 0);
    lv_obj_set_style_pad_left(button, 8, 0);
    lv_obj_set_style_pad_right(button, 8, 0);
    lv_obj_set_style_pad_top(button, 3, 0);
    lv_obj_set_style_pad_bottom(button, 4, 0);

    lv_obj_t* content = lv_obj_create(button);
    lv_obj_set_size(content, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_pad_all(content, 0, 0);
    lv_obj_set_style_pad_row(content, 0, 0);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* icon = lv_label_create(content);
    lv_label_set_text(icon, item.icon);
    lv_obj_set_style_text_color(icon, lv_color_hex(0xFFFFFF), 0);
    if (bodyFont) {
        lv_obj_set_style_text_font(icon, bodyFont, 0);
    }

    lv_obj_t* english = lv_label_create(content);
    lv_label_set_text(english, item.english);
    lv_label_set_long_mode(english, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(english, lv_pct(100));
    lv_obj_set_style_text_align(english, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(english, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_letter_space(english, -1, 0);
    if (titleFont) {
        lv_obj_set_style_text_font(english, titleFont, 0);
    }

    lv_obj_t* chinese = lv_label_create(content);
    lv_label_set_text(chinese, item.chinese);
    lv_label_set_long_mode(chinese, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(chinese, lv_pct(100));
    lv_obj_set_style_text_align(chinese, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(chinese, lv_color_hex(0xE6F0FF), 0);
    lv_obj_set_style_text_letter_space(chinese, -1, 0);
    if (bodyFont) {
        lv_obj_set_style_text_font(chinese, bodyFont, 0);
    }

    lv_obj_add_event_cb(
        button,
        menu_event_cb,
        LV_EVENT_CLICKED,
        reinterpret_cast<void*>(static_cast<std::uintptr_t>(item.target))
    );

    return button;
}

static void create_screen() {
    auto* manager = pet_ui::manager::UiManager::getInstance();
    g_screen = lv_obj_create(manager->getContentHost());
    manager->registerScreen(pet_ui::manager::ScreenType::Menu, &g_screen);
    lv_obj_add_event_cb(g_screen, screen_cleanup_cb, LV_EVENT_DELETE, nullptr);

    lv_obj_set_size(g_screen, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(g_screen, lv_color_hex(0x0657D8), 0);
    lv_obj_set_style_bg_grad_color(g_screen, lv_color_hex(0x0A3E9A), 0);
    lv_obj_set_style_bg_grad_dir(g_screen, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(g_screen, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(g_screen, 0, 0);
    lv_obj_set_style_pad_left(g_screen, 8, 0);
    lv_obj_set_style_pad_right(g_screen, 8, 0);
    lv_obj_set_style_pad_top(g_screen, 8, 0);
    lv_obj_set_style_pad_bottom(g_screen, 8, 0);
    lv_obj_set_style_pad_row(g_screen, 6, 0);
    lv_obj_set_flex_flow(g_screen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(g_screen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(g_screen, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* top_bar = lv_obj_create(g_screen);
    lv_obj_set_width(top_bar, lv_pct(100));
    lv_obj_set_height(top_bar, 24);
    lv_obj_set_style_bg_color(top_bar, lv_color_hex(0x4FA1FF), 0);
    lv_obj_set_style_bg_grad_color(top_bar, lv_color_hex(0x2A73E8), 0);
    lv_obj_set_style_bg_grad_dir(top_bar, LV_GRAD_DIR_HOR, 0);
    lv_obj_set_style_bg_opa(top_bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(top_bar, 1, 0);
    lv_obj_set_style_border_color(top_bar, lv_color_hex(0xA5D4FF), 0);
    lv_obj_set_style_radius(top_bar, 0, 0);
    lv_obj_set_style_pad_left(top_bar, 10, 0);
    lv_obj_set_style_pad_right(top_bar, 10, 0);
    lv_obj_set_style_pad_top(top_bar, 0, 0);
    lv_obj_set_style_pad_bottom(top_bar, 0, 0);
    lv_obj_set_flex_flow(top_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_bar, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(top_bar, LV_OBJ_FLAG_SCROLLABLE);

    g_top_left = lv_label_create(top_bar);
    lv_obj_set_style_text_color(g_top_left, lv_color_hex(0xFFFFFF), 0);
    if (manager->getBodyFont()) {
        lv_obj_set_style_text_font(g_top_left, manager->getBodyFont(), 0);
    }

    lv_obj_t* center_text = lv_label_create(top_bar);
    lv_label_set_text(center_text, "主菜单");
    lv_obj_set_style_text_color(center_text, lv_color_hex(0xFFFFFF), 0);
    if (manager->getTitleFont()) {
        lv_obj_set_style_text_font(center_text, manager->getTitleFont(), 0);
    }

    g_top_right = lv_label_create(top_bar);
    lv_obj_set_style_text_color(g_top_right, lv_color_hex(0xFFFFFF), 0);
    if (manager->getBodyFont()) {
        lv_obj_set_style_text_font(g_top_right, manager->getBodyFont(), 0);
    }
    refresh_top_bar_clock();

    lv_obj_t* grid = lv_obj_create(g_screen);
    lv_obj_set_width(grid, lv_pct(100));
    lv_obj_set_flex_grow(grid, 1);
    lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(grid, 0, 0);
    lv_obj_set_style_pad_all(grid, 4, 0);
    lv_obj_set_style_pad_row(grid, 8, 0);
    lv_obj_set_style_pad_column(grid, 10, 0);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(grid, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);

    g_menu_buttons.clear();
    for (const auto& item : kMenuItems) {
        g_menu_buttons.push_back(
            create_menu_tile(grid, item, manager->getTitleFont(), manager->getBodyFont())
        );
    }

    lv_obj_t* spacer = lv_obj_create(grid);
    lv_obj_set_size(spacer, 128, 70);
    lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(spacer, 0, 0);
    lv_obj_clear_flag(spacer, LV_OBJ_FLAG_SCROLLABLE);
}

}  // namespace

namespace pet_ui::screen::menu {

void load_screen() {
    auto* manager = pet_ui::manager::UiManager::getInstance();
    if (!g_screen) {
        create_screen();
    }
    refresh_top_bar_clock();

    manager->resetKeypadGroup();
    for (lv_obj_t* button : g_menu_buttons) {
        manager->addObjToGroup(button);
    }
    if (!g_menu_buttons.empty()) {
        lv_group_focus_obj(g_menu_buttons.front());
    }
    manager->loadContentScreen(pet_ui::manager::ScreenType::Menu, g_screen);
}

}  // namespace pet_ui::screen::menu
