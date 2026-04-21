#pragma once

#include "lvgl.h"
#include <string>

namespace pet_ui::screen::interaction {

struct StrategyControlsPanel {
    lv_obj_t* root = nullptr;
    lv_obj_t* modeValue = nullptr;
    lv_obj_t* quiet = nullptr;
    lv_obj_t* normal = nullptr;
    lv_obj_t* clingy = nullptr;
    lv_obj_t* reset = nullptr;
    lv_obj_t* toggleCheckin = nullptr;
    lv_obj_t* toggleTease = nullptr;
    lv_obj_t* toggleReminder = nullptr;
    lv_obj_t* toggleSilentNight = nullptr;
};

StrategyControlsPanel create_strategy_controls_panel(lv_obj_t* parent, const lv_font_t* titleFont, const lv_font_t* bodyFont);
void set_disturbance_mode(StrategyControlsPanel& panel, const std::string& mode);
void set_behavior_toggle(StrategyControlsPanel& panel, const std::string& key, const std::string& enabled);

}  // namespace pet_ui::screen::interaction
