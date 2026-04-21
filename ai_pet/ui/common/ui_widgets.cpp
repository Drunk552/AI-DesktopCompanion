#include "ui/common/ui_widgets.h"
#include "ui/common/ui_style.h"

namespace pet_ui::widgets {

lv_obj_t* create_menu_button(lv_obj_t* parent, const std::string& title, const std::string& subtitle, const lv_font_t* font) {
    lv_obj_t* button = lv_button_create(parent);
    style::style_button(button, lv_color_hex(0x2B3042));
    lv_obj_set_size(button, lv_pct(48), 88);

    lv_obj_t* container = lv_obj_create(button);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 0, 0);
    lv_obj_set_style_pad_row(container, 4, 0);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);

    style::create_text(container, title, font, lv_color_hex(0xFFFFFF));
    style::create_text(container, subtitle, font, lv_color_hex(0xBAC2DE));
    return button;
}

lv_obj_t* create_back_button(lv_obj_t* parent, const std::string& text, const lv_font_t* font) {
    lv_obj_t* button = lv_button_create(parent);
    style::style_button(button, lv_color_hex(0x6C7086));
    lv_obj_t* label = lv_label_create(button);
    lv_label_set_text(label, text.c_str());
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    if (font) {
        lv_obj_set_style_text_font(label, font, 0);
    }
    lv_obj_center(label);
    return button;
}

lv_obj_t* create_status_chip(lv_obj_t* parent, const std::string& text, lv_color_t bgColor, const lv_font_t* font) {
    lv_obj_t* chip = lv_obj_create(parent);
    lv_obj_set_size(chip, LV_SIZE_CONTENT, 28);
    lv_obj_set_style_bg_color(chip, bgColor, 0);
    lv_obj_set_style_bg_opa(chip, LV_OPA_90, 0);
    lv_obj_set_style_border_width(chip, 0, 0);
    lv_obj_set_style_radius(chip, 14, 0);
    lv_obj_set_style_pad_left(chip, 10, 0);
    lv_obj_set_style_pad_right(chip, 10, 0);
    lv_obj_set_style_pad_top(chip, 4, 0);
    lv_obj_set_style_pad_bottom(chip, 4, 0);
    lv_obj_clear_flag(chip, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* label = lv_label_create(chip);
    lv_label_set_text(label, text.c_str());
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    if (font) {
        lv_obj_set_style_text_font(label, font, 0);
    }
    lv_obj_center(label);
    return label;
}

}  // namespace pet_ui::widgets
