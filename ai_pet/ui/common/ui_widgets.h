#pragma once

#include "lvgl.h"
#include <string>

namespace pet_ui::widgets {

lv_obj_t* create_menu_button(lv_obj_t* parent, const std::string& title, const std::string& subtitle, const lv_font_t* font);
lv_obj_t* create_back_button(lv_obj_t* parent, const std::string& text, const lv_font_t* font);
lv_obj_t* create_status_chip(lv_obj_t* parent, const std::string& text, lv_color_t bgColor, const lv_font_t* font);

}  // namespace pet_ui::widgets
