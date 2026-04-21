#pragma once

#include "lvgl.h"
#include <string>

namespace pet_ui::style {

void style_screen_root(lv_obj_t* obj);
lv_obj_t* create_card(lv_obj_t* parent, lv_color_t bgColor = lv_color_hex(0x1F2435), lv_opa_t bgOpa = LV_OPA_80);
lv_obj_t* create_title(lv_obj_t* parent, const std::string& text, const lv_font_t* font);
lv_obj_t* create_text(lv_obj_t* parent, const std::string& text, const lv_font_t* font, lv_color_t color = lv_color_hex(0xD9E0EE));
void style_button(lv_obj_t* button, lv_color_t bgColor);

}  // namespace pet_ui::style
