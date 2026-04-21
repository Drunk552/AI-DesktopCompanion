#pragma once

#include "app/app_event_bus.h"
#include <string>

namespace pet_ui::screen::home {

void bind_event_bus(AppEventBus& eventBus);
void process_pending();
void load_screen();
void append_message(const std::string& text, bool isUser, bool isProactive = false, const std::string& proactiveType = std::string());
void set_thinking(bool thinking);
void set_persona_name(const std::string& name);
bool is_scroll_active();

}  // namespace pet_ui::screen::home
