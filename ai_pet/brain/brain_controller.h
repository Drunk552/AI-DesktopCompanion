#pragma once

#include "action/action_coordinator.h"
#include "app/app_event_bus.h"
#include "brain/event_router.h"
#include "brain/brain_types.h"
#include <atomic>
#include <functional>
#include <future>
#include <mutex>
#include <string>

namespace brain {

class ConversationOrchestrator;
class EmotionState;
class RelationState;

class BrainController {
public:
    BrainController(
        ConversationOrchestrator& orchestrator,
        EmotionState& emotionState,
        RelationState& relationState,
        action::ActionCoordinator& actionCoordinator
    );
    ~BrainController();

    void init(AppEventBus& eventBus, std::function<void()> stopRequestedCallback);
    void bindUIEvents();
    void syncUIState();
    void shutdown();

    void handleUserTextInput(const std::string& input);
    SessionTurnResult processUserTextSync(const std::string& input);
    void handlePerceptionEmotion(const std::string& emotion);
    void handleSystemIdleTimeout();
    bool isBusy() const;
    BrainState currentState() const;
    void setActiveMode(const std::string& mode);
    void setPersonaName(const std::string& personaName);

private:
    void startConversationTurn(const BrainInput& input);
    void publishTurnResult(const SessionTurnResult& result);
    void triggerProactiveBehavior(const BrainState& state);
    void triggerIdleBehavior(const BrainState& state);
    void refreshStateLocked();
    void emitStateChanged();
    void waitForPendingTurn();

    ConversationOrchestrator& orchestrator_;
    EmotionState& emotionState_;
    RelationState& relationState_;
    action::ActionCoordinator& actionCoordinator_;
    AppEventBus* eventBus_ = nullptr;
    EventRouter eventRouter_;
    std::function<void()> stopRequestedCallback_;
    std::future<void> pendingTurn_;
    mutable std::mutex stateMutex_;
    BrainState state_;
    std::string activeMode_ = "unknown";
    std::string personaName_ = "未命名";
    std::string lastDetectedEmotion_ = "平静";
};

}  // namespace brain
