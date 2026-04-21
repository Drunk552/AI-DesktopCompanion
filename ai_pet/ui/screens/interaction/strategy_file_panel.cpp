#include "ui/screens/interaction/strategy_file_panel.h"
#include "ui/common/ui_style.h"
#include "ui/common/ui_widgets.h"

namespace pet_ui::screen::interaction {

StrategyFilePanel create_strategy_file_panel(lv_obj_t* parent, const lv_font_t* titleFont, const lv_font_t* bodyFont) {
    StrategyFilePanel panel;
    panel.root = pet_ui::style::create_card(parent, lv_color_hex(0x2C364C), LV_OPA_80);
    pet_ui::style::create_title(panel.root, "策略文件", titleFont);
    pet_ui::style::create_text(panel.root, "支持导出、恢复默认，以及分粒度导入策略文件。", bodyFont);

    lv_obj_t* row = lv_obj_create(panel.root);
    lv_obj_set_width(row, lv_pct(100));
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_style_pad_column(row, 8, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    panel.exportBtn = pet_ui::widgets::create_back_button(row, "导出策略", bodyFont);
    panel.importFullBtn = pet_ui::widgets::create_back_button(row, "导入全部", bodyFont);
    panel.importTogglesBtn = pet_ui::widgets::create_back_button(row, "只导入开关", bodyFont);
    panel.importStatsBtn = pet_ui::widgets::create_back_button(row, "只导入统计", bodyFont);
    panel.restoreDefaultsBtn = pet_ui::widgets::create_back_button(row, "恢复默认", bodyFont);
    return panel;
}

}  // namespace pet_ui::screen::interaction
