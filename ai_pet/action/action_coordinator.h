#pragma once

#include <memory>

class AppEventBus;

namespace action {

class ChatActionController;
class BehaviorPolicy;
class PetActionController;
class NotificationActionController;
class StrategyStateService;

class ActionCoordinator {
public:
    explicit ActionCoordinator(AppEventBus& eventBus);
    ~ActionCoordinator();

    void setDisturbanceMode(const std::string& mode);
    void setPersonaProfile(const std::string& personaName);
    ChatActionController& chat();
    BehaviorPolicy& behaviorPolicy();
    PetActionController& pet();
    NotificationActionController& notification();

private:
    std::unique_ptr<StrategyStateService> strategyStateService_;
    std::unique_ptr<BehaviorPolicy> behaviorPolicy_;
    std::unique_ptr<ChatActionController> chat_;
    std::unique_ptr<PetActionController> pet_;
    std::unique_ptr<NotificationActionController> notification_;
};

}  // namespace action
