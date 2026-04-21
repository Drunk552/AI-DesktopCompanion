#include "action/action_coordinator.h"
#include "action/behavior_policy.h"
#include "action/chat_action_controller.h"
#include "action/notification_action_controller.h"
#include "action/pet_action_controller.h"
#include "action/strategy_state_service.h"
#include "app/app_event_bus.h"

namespace action {

ActionCoordinator::ActionCoordinator(AppEventBus& eventBus)
    : strategyStateService_(std::make_unique<StrategyStateService>())
    , behaviorPolicy_(std::make_unique<BehaviorPolicy>(*strategyStateService_))
    , chat_(std::make_unique<ChatActionController>(eventBus, *behaviorPolicy_))
    , pet_(std::make_unique<PetActionController>(eventBus))
    , notification_(std::make_unique<NotificationActionController>(eventBus, *behaviorPolicy_)) {}

ActionCoordinator::~ActionCoordinator() = default;

void ActionCoordinator::setDisturbanceMode(const std::string& mode) {
    behaviorPolicy_->setDisturbanceMode(mode);
}

void ActionCoordinator::setPersonaProfile(const std::string& personaName) {
    behaviorPolicy_->setPersonaProfile(personaName);
}

ChatActionController& ActionCoordinator::chat() { return *chat_; }
BehaviorPolicy& ActionCoordinator::behaviorPolicy() { return *behaviorPolicy_; }
PetActionController& ActionCoordinator::pet() { return *pet_; }
NotificationActionController& ActionCoordinator::notification() { return *notification_; }

}  // namespace action
