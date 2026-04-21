#pragma once

#include <string>

namespace pet_ui::overlay::notification {

void create();
void show(const std::string& text);
void tick();

}  // namespace pet_ui::overlay::notification
