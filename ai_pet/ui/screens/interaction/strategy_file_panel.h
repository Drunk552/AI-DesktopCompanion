#pragma once

#include "lvgl.h"

namespace pet_ui::screen::interaction {

struct StrategyFilePanel {
    lv_obj_t* root = nullptr;
    lv_obj_t* exportBtn = nullptr;
    lv_obj_t* importFullBtn = nullptr;
    lv_obj_t* importTogglesBtn = nullptr;
    lv_obj_t* importStatsBtn = nullptr;
    lv_obj_t* restoreDefaultsBtn = nullptr;
};

StrategyFilePanel create_strategy_file_panel(lv_obj_t* parent, const lv_font_t* titleFont, const lv_font_t* bodyFont);

}  // namespace pet_ui::screen::interaction
