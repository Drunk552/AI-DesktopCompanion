#pragma once

#include "app/app_event_bus.h"
#include "lvgl.h"
#include <string>

namespace ui_input_handler {

void bind_emit_on_click(lv_obj_t* obj, AppEventBus& eventBus, const std::string& eventType, const std::string& data);
void bind_text_submit(lv_obj_t* trigger, lv_event_code_t eventCode, lv_obj_t* textarea, AppEventBus& eventBus);

}  // namespace ui_input_handler
