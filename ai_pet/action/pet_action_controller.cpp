#include "action/pet_action_controller.h"
#include "action/action_events.h"
#include "app/app_event_bus.h"

namespace action {

PetActionController::PetActionController(AppEventBus& eventBus)
    : eventBus_(eventBus) {}

void PetActionController::updateEmotion(const std::string& emotion) {
    eventBus_.emitTyped(events::kPetEmotion, events::PetStateEvent{emotion});
}

void PetActionController::updateAffection(int affinity) {
    eventBus_.emitTyped(events::kPetAffection, events::PetStateEvent{std::to_string(affinity)});
}

void PetActionController::updateRelationship(const std::string& relationship) {
    eventBus_.emitTyped(events::kPetRelationship, events::PetStateEvent{relationship});
}

}  // namespace action
