#include "brain/brain_controller.h"
#include "action/chat_action_controller.h"
#include "action/action_events.h"
#include "action/notification_action_controller.h"
#include "action/pet_action_controller.h"
#include "action/behavior_policy.h"
#include "brain/brain_events.h"
#include "brain/conversation_orchestrator.h"
#include "brain/emotion_state.h"
#include "brain/relation_state.h"
#include "shared/logger/logger.h"

namespace brain {

BrainController::BrainController(
    ConversationOrchestrator& orchestrator,
    EmotionState& emotionState,
    RelationState& relationState,
    action::ActionCoordinator& actionCoordinator
)
    : orchestrator_(orchestrator)
    , emotionState_(emotionState)
    , relationState_(relationState)
    , actionCoordinator_(actionCoordinator) {}

BrainController::~BrainController() {
    shutdown();
}

void BrainController::init(AppEventBus& eventBus, std::function<void()> stopRequestedCallback) {
    eventBus_ = &eventBus;
    stopRequestedCallback_ = std::move(stopRequestedCallback);
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        refreshStateLocked();
    }
    bindUIEvents();
}

void BrainController::bindUIEvents() {
    if (!eventBus_) {
        return;
    }

    eventRouter_.bind(*eventBus_, {
        [this](const std::string& input) { handleUserTextInput(input); },
        [this](const std::string& emotion) { handlePerceptionEmotion(emotion); },
        [this]() { handleSystemIdleTimeout(); },
    });
}

void BrainController::syncUIState() {
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        refreshStateLocked();
    }

    if (eventBus_) {
        BrainState state = currentState();
        actionCoordinator_.pet().updateEmotion(state.emotion);
        actionCoordinator_.pet().updateAffection(state.affinity);
        actionCoordinator_.pet().updateRelationship(state.relationship);
        emitStateChanged();
    }
}

void BrainController::shutdown() {
    orchestrator_.cancelPending();
    waitForPendingTurn();
    std::lock_guard<std::mutex> lock(stateMutex_);
    refreshStateLocked();
}

bool BrainController::isBusy() const {
    return orchestrator_.isBusy();
}

BrainState BrainController::currentState() const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return state_;
}

void BrainController::setActiveMode(const std::string& mode) {
    std::lock_guard<std::mutex> lock(stateMutex_);
    activeMode_ = mode.empty() ? "unknown" : mode;
    refreshStateLocked();
}

void BrainController::setPersonaName(const std::string& personaName) {
    std::lock_guard<std::mutex> lock(stateMutex_);
    personaName_ = personaName.empty() ? "未命名" : personaName;
    refreshStateLocked();
}

void BrainController::handleUserTextInput(const std::string& input) {
    if (input == "quit" || input == "exit") {
        orchestrator_.cancelPending();
        {
            std::lock_guard<std::mutex> lock(stateMutex_);
            refreshStateLocked();
        }
        emitStateChanged();
        if (stopRequestedCallback_) {
            stopRequestedCallback_();
        }
        return;
    }

    if (input.empty()) {
        return;
    }

    actionCoordinator_.behaviorPolicy().recordUserInteraction();

    if (!orchestrator_.tryBeginTurn()) {
        LOGW("Brain", "AI 正在处理中，请稍候");
        actionCoordinator_.notification().show("AI 还在思考上一句，先别急。");
        return;
    }

    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        refreshStateLocked();
    }
    if (eventBus_) {
        eventBus_->emitTyped(events::kBrainTurnStarted, events::BrainTurnStartedEvent{});
        actionCoordinator_.chat().startThinking();
        emitStateChanged();
    }

    waitForPendingTurn();
    startConversationTurn({input, "user"});
}

SessionTurnResult BrainController::processUserTextSync(const std::string& input) {
    SessionTurnResult result;
    if (input.empty()) {
        return result;
    }

    actionCoordinator_.behaviorPolicy().recordUserInteraction();

    if (!orchestrator_.tryBeginTurn()) {
        LOGW("Brain", "AI 正在处理中，请稍候");
        if (eventBus_) {
            actionCoordinator_.notification().show("AI 还在思考上一句，先别急。");
        }
        return result;
    }

    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        refreshStateLocked();
    }

    if (eventBus_) {
        eventBus_->emitTyped(events::kBrainTurnStarted, events::BrainTurnStartedEvent{});
        actionCoordinator_.chat().startThinking();
        emitStateChanged();
    }

    waitForPendingTurn();

    try {
        result = orchestrator_.handleTurn(input, 180000, false);
        publishTurnResult(result);
    } catch (const std::exception& e) {
        LOGE("Brain", "同步处理消息异常: " + std::string(e.what()));
        {
            std::lock_guard<std::mutex> lock(stateMutex_);
            refreshStateLocked();
        }
        if (eventBus_) {
            actionCoordinator_.chat().stopThinking();
            actionCoordinator_.chat().showSystemError("系统错误: " + std::string(e.what()));
            emitStateChanged();
        }
    } catch (...) {
        LOGE("Brain", "同步处理消息发生未知异常");
        {
            std::lock_guard<std::mutex> lock(stateMutex_);
            refreshStateLocked();
        }
        if (eventBus_) {
            actionCoordinator_.chat().stopThinking();
            actionCoordinator_.chat().showSystemError("系统错误");
            emitStateChanged();
        }
    }

    return result;
}

void BrainController::handlePerceptionEmotion(const std::string& emotion) {
    if (emotion.empty()) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        emotionState_.updateDetectedEmotion(emotion);
        lastDetectedEmotion_ = emotion;
        refreshStateLocked();
    }
    if (eventBus_) {
        eventBus_->emit(events::kBrainEmotionChanged, emotion);
        actionCoordinator_.pet().updateEmotion(emotion);
        emitStateChanged();
    }
}

void BrainController::handleSystemIdleTimeout() {
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        refreshStateLocked();
    }

    if (!eventBus_) {
        return;
    }

    triggerIdleBehavior(currentState());
    emitStateChanged();
}

void BrainController::startConversationTurn(const BrainInput& input) {
    pendingTurn_ = std::async(std::launch::async, [this, input]() {
        try {
            const SessionTurnResult result = orchestrator_.handleTurn(input.text, 180000, false);
            publishTurnResult(result);
        } catch (const std::exception& e) {
            LOGE("Brain", "处理消息异常: " + std::string(e.what()));
            {
                std::lock_guard<std::mutex> lock(stateMutex_);
                refreshStateLocked();
            }
            if (eventBus_) {
                actionCoordinator_.chat().stopThinking();
                actionCoordinator_.chat().showSystemError("系统错误: " + std::string(e.what()));
                emitStateChanged();
            }
        } catch (...) {
            LOGE("Brain", "处理消息发生未知异常");
            {
                std::lock_guard<std::mutex> lock(stateMutex_);
                refreshStateLocked();
            }
            if (eventBus_) {
                actionCoordinator_.chat().stopThinking();
                actionCoordinator_.chat().showSystemError("系统错误");
                emitStateChanged();
            }
        }
    });
}

void BrainController::publishTurnResult(const SessionTurnResult& result) {
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        refreshStateLocked();
    }

    if (!eventBus_) {
        return;
    }

    actionCoordinator_.chat().stopThinking();

    if (result.success) {
        actionCoordinator_.chat().showReply(result.reply);
        actionCoordinator_.pet().updateEmotion(result.emotion);
        actionCoordinator_.pet().updateAffection(result.affinity);
        actionCoordinator_.pet().updateRelationship(result.affinityLevel);
        eventBus_->emitTyped(events::kBrainTurnCompleted, events::BrainTurnCompletedEvent{result.accepted, result.success, result.reply});
        emitStateChanged();
        return;
    }

    if (!result.accepted) {
        eventBus_->emitTyped(events::kBrainTurnCompleted, events::BrainTurnCompletedEvent{result.accepted, result.success, result.reply});
        emitStateChanged();
        return;
    }

    actionCoordinator_.chat().showReply("……抱歉，网络好像有点问题");
    actionCoordinator_.notification().show("AI 请求失败，请稍后再试。");
    eventBus_->emitTyped(events::kBrainTurnCompleted, events::BrainTurnCompletedEvent{result.accepted, result.success, result.reply});
    emitStateChanged();
}

void BrainController::refreshStateLocked() {
    state_.emotion = emotionState_.currentEmotion();
    state_.affinity = relationState_.currentAffinity();
    state_.relationship = relationState_.currentAffinityLevel();
    state_.busy = orchestrator_.isBusy();
    state_.personaName = personaName_;
    state_.lastProactiveBehavior = actionCoordinator_.behaviorPolicy().lastProactiveBehaviorLabel();
    state_.lastUserInteractionAt = actionCoordinator_.behaviorPolicy().lastUserInteractionLabel();
    state_.lastDetectedEmotion = lastDetectedEmotion_;
    state_.recentEmotionTrend = relationState_.recentEmotionTrend();
    state_.disturbanceMode = actionCoordinator_.behaviorPolicy().disturbanceModeLabel();
    state_.personaStyle = actionCoordinator_.behaviorPolicy().personaStyleLabel();
    state_.activeMode = activeMode_;
    state_.quietHourActive = actionCoordinator_.behaviorPolicy().isQuietHourActive();
    state_.proactiveReady = actionCoordinator_.behaviorPolicy().canTriggerIdleBehavior(state_);
}

void BrainController::emitStateChanged() {
    if (!eventBus_) {
        return;
    }
    eventBus_->emitTyped(events::kBrainStateChanged, BrainStateChangedEvent{currentState()});
}

void BrainController::triggerProactiveBehavior(const BrainState& state) {
    if (!eventBus_ || !actionCoordinator_.behaviorPolicy().shouldTriggerProactiveBehavior(state)) {
        return;
    }

    const auto proactiveType = actionCoordinator_.behaviorPolicy().selectProactiveBehavior(state);
    const std::string proactiveMessage = actionCoordinator_.behaviorPolicy().buildProactiveMessage(proactiveType, state);
    actionCoordinator_.behaviorPolicy().markProactiveBehavior(proactiveType);

    switch (proactiveType) {
        case action::ProactiveBehaviorType::Care:
            eventBus_->emitTyped(action::events::kBehaviorProactiveCare, action::events::ProactiveBehaviorEvent{proactiveMessage, "care"});
            break;
        case action::ProactiveBehaviorType::CheckIn:
            eventBus_->emitTyped(action::events::kBehaviorProactiveCheckIn, action::events::ProactiveBehaviorEvent{proactiveMessage, "check_in"});
            break;
        case action::ProactiveBehaviorType::Tease:
            eventBus_->emitTyped(action::events::kBehaviorProactiveTease, action::events::ProactiveBehaviorEvent{proactiveMessage, "tease"});
            break;
        case action::ProactiveBehaviorType::Reminder:
            eventBus_->emitTyped(action::events::kBehaviorProactiveReminder, action::events::ProactiveBehaviorEvent{proactiveMessage, "reminder"});
            break;
    }

    actionCoordinator_.notification().show(proactiveMessage);
}

void BrainController::triggerIdleBehavior(const BrainState& state) {
    if (!eventBus_ || !actionCoordinator_.behaviorPolicy().shouldTriggerIdleBehavior(state)) {
        return;
    }

    const auto proactiveType = actionCoordinator_.behaviorPolicy().selectIdleBehavior(state);
    const std::string proactiveMessage = actionCoordinator_.behaviorPolicy().buildProactiveMessage(proactiveType, state);
    actionCoordinator_.behaviorPolicy().markProactiveBehavior(proactiveType);

    switch (proactiveType) {
        case action::ProactiveBehaviorType::Care:
            eventBus_->emitTyped(action::events::kBehaviorProactiveCare, action::events::ProactiveBehaviorEvent{proactiveMessage, "care"});
            break;
        case action::ProactiveBehaviorType::CheckIn:
            eventBus_->emitTyped(action::events::kBehaviorProactiveCheckIn, action::events::ProactiveBehaviorEvent{proactiveMessage, "check_in"});
            break;
        case action::ProactiveBehaviorType::Tease:
            eventBus_->emitTyped(action::events::kBehaviorProactiveTease, action::events::ProactiveBehaviorEvent{proactiveMessage, "tease"});
            break;
        case action::ProactiveBehaviorType::Reminder:
            eventBus_->emitTyped(action::events::kBehaviorProactiveReminder, action::events::ProactiveBehaviorEvent{proactiveMessage, "reminder"});
            break;
    }

    actionCoordinator_.notification().show(proactiveMessage);
}

void BrainController::waitForPendingTurn() {
    if (pendingTurn_.valid()) {
        pendingTurn_.wait();
    }
}

}  // namespace brain
