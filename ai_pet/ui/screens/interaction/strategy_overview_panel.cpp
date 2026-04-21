#include "ui/screens/interaction/strategy_overview_panel.h"
#include "ui/common/ui_style.h"

namespace pet_ui::screen::interaction {

namespace {

void set_label(lv_obj_t* label, const std::string& prefix, const std::string& value) {
    if (label) {
        lv_label_set_text(label, (prefix + value).c_str());
    }
}

}  // namespace

StrategyOverviewPanel create_strategy_overview_panel(lv_obj_t* parent, const lv_font_t* titleFont, const lv_font_t* bodyFont) {
    StrategyOverviewPanel panel;
    panel.root = pet_ui::style::create_card(parent, lv_color_hex(0x3A2236), LV_OPA_80);
    pet_ui::style::create_title(panel.root, "主动对话模式", titleFont);
    pet_ui::style::create_text(panel.root, "……你今天很安静。", bodyFont);
    panel.personaValue = pet_ui::style::create_text(panel.root, "人格风格：默认风格", bodyFont, lv_color_hex(0xF9E2AF));
    panel.strategyValue = pet_ui::style::create_text(panel.root, "主动策略：正常，冷却 45 秒", bodyFont, lv_color_hex(0xD9E0EE));
    panel.lastBehaviorValue = pet_ui::style::create_text(panel.root, "最近主动行为：无", bodyFont, lv_color_hex(0xD9E0EE));
    panel.lastInteractionValue = pet_ui::style::create_text(panel.root, "最近互动：从未", bodyFont, lv_color_hex(0xD9E0EE));
    panel.quietHourValue = pet_ui::style::create_text(panel.root, "安静时段：否", bodyFont, lv_color_hex(0xD9E0EE));
    panel.statsValue = pet_ui::style::create_text(panel.root, "行为统计：关怀 0 / 问候 0 / 逗弄 0 / 提醒 0", bodyFont, lv_color_hex(0xD9E0EE));
    panel.pathValue = pet_ui::style::create_text(panel.root, "策略文件：data/strategy_settings_export.json", bodyFont, lv_color_hex(0xD9E0EE));
    panel.importResultValue = pet_ui::style::create_text(panel.root, "导入结果：未执行", bodyFont, lv_color_hex(0xD9E0EE));
    panel.importErrorValue = pet_ui::style::create_text(panel.root, "错误详情：无", bodyFont, lv_color_hex(0xD9E0EE));
    return panel;
}

void set_persona_style(StrategyOverviewPanel& panel, const std::string& value) { set_label(panel.personaValue, "人格风格：", value); }
void set_strategy_summary(StrategyOverviewPanel& panel, const std::string& value) { if (panel.strategyValue) lv_label_set_text(panel.strategyValue, value.c_str()); }
void set_last_behavior(StrategyOverviewPanel& panel, const std::string& value) { set_label(panel.lastBehaviorValue, "最近主动行为：", value); }
void set_last_interaction(StrategyOverviewPanel& panel, const std::string& value) { set_label(panel.lastInteractionValue, "最近互动：", value); }
void set_quiet_hour(StrategyOverviewPanel& panel, const std::string& value) { set_label(panel.quietHourValue, "安静时段：", value); }
void set_behavior_stats(StrategyOverviewPanel& panel, const std::string& value) { set_label(panel.statsValue, "行为统计：", value); }
void set_strategy_file_path(StrategyOverviewPanel& panel, const std::string& value) { set_label(panel.pathValue, "策略文件：", value); }
void set_import_result(StrategyOverviewPanel& panel, const std::string& value) { set_label(panel.importResultValue, "导入结果：", value); }
void set_import_error(StrategyOverviewPanel& panel, const std::string& value) { set_label(panel.importErrorValue, "错误详情：", value); }

}  // namespace pet_ui::screen::interaction
