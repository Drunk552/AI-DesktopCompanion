#include "ui/screens/interaction/strategy_controls_panel.h"
#include "ui/common/ui_style.h"
#include "ui/common/ui_widgets.h"

namespace pet_ui::screen::interaction {

namespace {

void update_toggle_text(lv_obj_t* label, const std::string& prefix, const std::string& enabled) {
    if (label) {
        lv_label_set_text(label, (prefix + (enabled == "1" ? "开" : "关")).c_str());
    }
}

}  // namespace

StrategyControlsPanel create_strategy_controls_panel(lv_obj_t* parent, const lv_font_t* titleFont, const lv_font_t* bodyFont) {
    StrategyControlsPanel panel;
    panel.root = pet_ui::style::create_card(parent);
    pet_ui::style::create_title(panel.root, "打扰模式", titleFont);
    panel.modeValue = pet_ui::style::create_text(panel.root, "当前模式：正常", bodyFont, lv_color_hex(0xF9E2AF));

    lv_obj_t* modeRow = lv_obj_create(panel.root);
    lv_obj_set_width(modeRow, lv_pct(100));
    lv_obj_set_style_bg_opa(modeRow, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(modeRow, 0, 0);
    lv_obj_set_style_pad_all(modeRow, 0, 0);
    lv_obj_set_style_pad_column(modeRow, 8, 0);
    lv_obj_set_flex_flow(modeRow, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_clear_flag(modeRow, LV_OBJ_FLAG_SCROLLABLE);

    panel.quiet = pet_ui::widgets::create_back_button(modeRow, "安静", bodyFont);
    panel.normal = pet_ui::widgets::create_back_button(modeRow, "正常", bodyFont);
    panel.clingy = pet_ui::widgets::create_back_button(modeRow, "粘人", bodyFont);
    panel.reset = pet_ui::widgets::create_back_button(modeRow, "重置策略", bodyFont);

    lv_obj_t* behaviorRow = lv_obj_create(panel.root);
    lv_obj_set_width(behaviorRow, lv_pct(100));
    lv_obj_set_style_bg_opa(behaviorRow, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(behaviorRow, 0, 0);
    lv_obj_set_style_pad_all(behaviorRow, 0, 0);
    lv_obj_set_style_pad_column(behaviorRow, 8, 0);
    lv_obj_set_flex_flow(behaviorRow, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_clear_flag(behaviorRow, LV_OBJ_FLAG_SCROLLABLE);

    panel.toggleCheckin = pet_ui::widgets::create_back_button(behaviorRow, "问候：开", bodyFont);
    panel.toggleTease = pet_ui::widgets::create_back_button(behaviorRow, "逗弄：开", bodyFont);
    panel.toggleReminder = pet_ui::widgets::create_back_button(behaviorRow, "提醒：开", bodyFont);
    panel.toggleSilentNight = pet_ui::widgets::create_back_button(behaviorRow, "夜间静默：关", bodyFont);
    return panel;
}

void set_disturbance_mode(StrategyControlsPanel& panel, const std::string& mode) {
    if (!panel.modeValue) {
        return;
    }
    if (mode == "quiet") {
        lv_label_set_text(panel.modeValue, "当前模式：安静");
    } else if (mode == "clingy") {
        lv_label_set_text(panel.modeValue, "当前模式：粘人");
    } else {
        lv_label_set_text(panel.modeValue, "当前模式：正常");
    }
}

void set_behavior_toggle(StrategyControlsPanel& panel, const std::string& key, const std::string& enabled) {
    if (key == "checkin") update_toggle_text(panel.toggleCheckin, "问候：", enabled);
    else if (key == "tease") update_toggle_text(panel.toggleTease, "逗弄：", enabled);
    else if (key == "reminder") update_toggle_text(panel.toggleReminder, "提醒：", enabled);
    else if (key == "silent_night") update_toggle_text(panel.toggleSilentNight, "夜间静默：", enabled);
}

}  // namespace pet_ui::screen::interaction
