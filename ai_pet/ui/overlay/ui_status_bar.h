#pragma once

#include "app/app_event_bus.h"
#include "brain/brain_types.h"
#include <string>

namespace pet_ui::overlay::status_bar {

void create();
void bind_event_bus(AppEventBus& eventBus);
void process_pending();
void update_persona_name(const std::string& name);
void update_emotion(const std::string& emotion);
void update_affection(const std::string& affection);
void update_relationship(const std::string& relationship);
void update_strategy(const std::string& strategy);
void apply_brain_state(const brain::BrainState& state);

}  // namespace pet_ui::overlay::status_bar
