#include "brain/conversation_orchestrator.h"
#include "intelligence/prompt/prompt.h"
#include "brain/emotion_state.h"
#include "brain/relation_state.h"
#include "shared/logger/logger.h"
#include "intelligence/memory/memory_db.h"
#include <chrono>

namespace brain {

ConversationOrchestrator::ConversationOrchestrator(
    GemmaAI& ai,
    MemoryDB& memory,
    PersonaLoader& persona,
    EmotionState& emotionState,
    RelationState& relationState
)
    : ai_(ai)
    , memory_(memory)
    , persona_(persona)
    , emotionState_(emotionState)
    , relationState_(relationState) {}

SessionTurnResult ConversationOrchestrator::handleTurn(const std::string& input, int timeoutMs, bool reserveTurn) {
    SessionTurnResult result;
    if (input.empty()) {
        return result;
    }

    if (reserveTurn && aiRunning_.exchange(true)) {
        return result;
    }

    result.accepted = true;

    try {
        cancelled_.store(false);
        result.emotion = emotionState_.currentEmotion();

        const RelationSnapshot relation = relationState_.snapshot();
        PromptContext promptContext;
        promptContext.userInput = input;
        promptContext.emotion = result.emotion;
        promptContext.context = memory_.getContext(5);
        promptContext.affinity = relation.affinity;
        promptContext.emotionTrend = memory_.getEmotionTrend();
        promptContext.persona = getPersonaPtr();
        const std::string prompt = buildPrompt(promptContext);

        auto future = ai_.chatAsync(prompt);
        std::string reply;

        if (waitForAI(future, reply, timeoutMs)) {
            if (reply.empty()) {
                reply = "……";
            }

            memory_.saveChat(input, reply, result.emotion);
            memory_.recordEmotion(result.emotion);
            memory_.incrementChatCount();
            relationState_.updateAfterConversation(result.emotion, input);

            result.success = true;
            result.reply = reply;
        }
    } catch (const std::exception& e) {
        LOGE("ConversationOrchestrator", "处理消息异常: " + std::string(e.what()));
    } catch (...) {
        LOGE("ConversationOrchestrator", "处理消息发生未知异常");
    }

    const RelationSnapshot relation = relationState_.snapshot();
    result.affinity = relation.affinity;
    result.affinityLevel = relation.affinityLevel;
    aiRunning_.store(false);
    return result;
}

bool ConversationOrchestrator::tryBeginTurn() {
    return !aiRunning_.exchange(true);
}

void ConversationOrchestrator::cancelPending() {
    cancelled_.store(true);
    ai_.cancel();
}

bool ConversationOrchestrator::isBusy() const {
    return aiRunning_.load();
}

bool ConversationOrchestrator::waitForAI(std::future<std::string>& future, std::string& result, int timeoutMs) {
    const auto status = future.wait_for(std::chrono::milliseconds(timeoutMs));
    if (status == std::future_status::ready) {
        try {
            result = future.get();
            return true;
        } catch (const std::exception& e) {
            LOGE("ConversationOrchestrator", "AI 调用异常: " + std::string(e.what()));
            return false;
        }
    }

    if (status == std::future_status::timeout) {
        LOGW("ConversationOrchestrator", "AI 调用超时");
        cancelled_.store(true);
        ai_.cancel();
    }
    return false;
}

const PersonaData* ConversationOrchestrator::getPersonaPtr() const {
    return persona_.isLoaded() ? &persona_.getData() : nullptr;
}

}  // namespace brain
