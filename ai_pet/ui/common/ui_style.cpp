#include "ui/common/ui_style.h"

namespace pet_ui::style {

void style_screen_root(lv_obj_t* obj) {
    lv_obj_set_size(obj, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 12, 0);
    lv_obj_set_style_pad_row(obj, 10, 0);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

lv_obj_t* create_card(lv_obj_t* parent, lv_color_t bgColor, lv_opa_t bgOpa) {
    lv_obj_t* card = lv_obj_create(parent);
    lv_obj_set_width(card, lv_pct(100));
    lv_obj_set_style_bg_color(card, bgColor, 0);
    lv_obj_set_style_bg_opa(card, bgOpa, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_radius(card, 18, 0);
    lv_obj_set_style_pad_all(card, 14, 0);
    lv_obj_set_style_pad_row(card, 8, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    return card;
}

lv_obj_t* create_title(lv_obj_t* parent, const std::string& text, const lv_font_t* font) {
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, text.c_str());
    lv_obj_set_style_text_color(label, lv_color_hex(0xF5E0DC), 0);
    if (font) {
        lv_obj_set_style_text_font(label, font, 0);
    }
    return label;
}

lv_obj_t* create_text(lv_obj_t* parent, const std::string& text, const lv_font_t* font, lv_color_t color) {
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, text.c_str());
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label, lv_pct(100));
    lv_obj_set_style_text_color(label, color, 0);
    if (font) {
        lv_obj_set_style_text_font(label, font, 0);
    }
    return label;
}

void style_button(lv_obj_t* button, lv_color_t bgColor) {
    lv_obj_set_style_bg_color(button, bgColor, 0);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(button, 0, 0);
    lv_obj_set_style_radius(button, 16, 0);
    lv_obj_set_style_shadow_width(button, 0, 0);
    lv_obj_set_style_pad_all(button, 12, 0);
}

}  // namespace pet_ui::style
