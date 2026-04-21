#pragma once

#include "app/app_event_bus.h"
#include "app/settings_events.h"
#include <string>

namespace pet_ui::screen::interaction {

void load_screen();
void bind_event_bus(AppEventBus& eventBus);
void process_pending();
void set_disturbance_mode(const std::string& mode);
void set_persona_style(const std::string& personaStyle);
void set_strategy_summary(const std::string& strategySummary);
void set_last_behavior(const std::string& behavior);
void set_last_interaction(const std::string& interaction);
void set_quiet_hour(const std::string& quietHour);
void set_behavior_stats(const std::string& stats);
void set_behavior_toggle(const std::string& key, const std::string& enabled);
void set_strategy_file_path(const std::string& path);
void set_import_result(const std::string& result);
void set_import_error(const std::string& error);
void apply_settings_snapshot(const ::app::settings::events::SettingsStateSnapshot& snapshot);

}  // namespace pet_ui::screen::interaction
