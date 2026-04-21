#pragma once

#include "app/app_event_bus.h"

namespace pet_ui::app {

bool init(int width, int height, AppEventBus* eventBus);
bool tick();
void shutdown();

}  // namespace pet_ui::app
