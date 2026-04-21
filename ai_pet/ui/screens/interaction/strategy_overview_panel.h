#pragma once

#include "lvgl.h"
#include <string>

namespace pet_ui::screen::interaction {

struct StrategyOverviewPanel {
    lv_obj_t* root = nullptr;
    lv_obj_t* personaValue = nullptr;
    lv_obj_t* strategyValue = nullptr;
    lv_obj_t* lastBehaviorValue = nullptr;
    lv_obj_t* lastInteractionValue = nullptr;
    lv_obj_t* quietHourValue = nullptr;
    lv_obj_t* statsValue = nullptr;
    lv_obj_t* pathValue = nullptr;
    lv_obj_t* importResultValue = nullptr;
    lv_obj_t* importErrorValue = nullptr;
};

StrategyOverviewPanel create_strategy_overview_panel(lv_obj_t* parent, const lv_font_t* titleFont, const lv_font_t* bodyFont);
void set_persona_style(StrategyOverviewPanel& panel, const std::string& value);
void set_strategy_summary(StrategyOverviewPanel& panel, const std::string& value);
void set_last_behavior(StrategyOverviewPanel& panel, const std::string& value);
void set_last_interaction(StrategyOverviewPanel& panel, const std::string& value);
void set_quiet_hour(StrategyOverviewPanel& panel, const std::string& value);
void set_behavior_stats(StrategyOverviewPanel& panel, const std::string& value);
void set_strategy_file_path(StrategyOverviewPanel& panel, const std::string& value);
void set_import_result(StrategyOverviewPanel& panel, const std::string& value);
void set_import_error(StrategyOverviewPanel& panel, const std::string& value);

}  // namespace pet_ui::screen::interaction
