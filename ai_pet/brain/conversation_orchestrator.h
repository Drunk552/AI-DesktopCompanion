#pragma once

#include "brain/brain_types.h"
#include "intelligence/llm/gemma.h"
#include "intelligence/persona/persona_loader.h"
#include <atomic>
#include <future>
#include <string>

class MemoryDB;

namespace brain {

class EmotionState;
class RelationState;

class ConversationOrchestrator {
public:
    ConversationOrchestrator(
        GemmaAI& ai,
        MemoryDB& memory,
        PersonaLoader& persona,
        EmotionState& emotionState,
        RelationState& relationState
    );

    SessionTurnResult handleTurn(const std::string& input, int timeoutMs, bool reserveTurn);
    bool tryBeginTurn();
    void cancelPending();
    bool isBusy() const;

private:
    bool waitForAI(std::future<std::string>& future, std::string& result, int timeoutMs);
    const PersonaData* getPersonaPtr() const;

    GemmaAI& ai_;
    MemoryDB& memory_;
    PersonaLoader& persona_;
    EmotionState& emotionState_;
    RelationState& relationState_;
    std::atomic<bool> aiRunning_{false};
    std::atomic<bool> cancelled_{false};
};

}  // namespace brain
