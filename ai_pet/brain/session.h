#pragma once

#include "brain/brain_types.h"
#include "intelligence/llm/gemma.h"
#include "intelligence/memory/memory_db.h"
#include "intelligence/persona/persona_loader.h"
#include <string>

namespace brain {

class ConversationOrchestrator;
class EmotionState;
class RelationState;

class Session {
public:
    Session(GemmaAI& ai, MemoryDB& memory, PersonaLoader& persona);
    ~Session();

    void initialize(const std::string& personasDir, const std::string& personaName = "");
    std::string personaName() const;
    ConversationOrchestrator& orchestrator();
    const ConversationOrchestrator& orchestrator() const;
    EmotionState& emotionState();
    const EmotionState& emotionState() const;
    RelationState& relationState();
    const RelationState& relationState() const;

private:
    void loadPersona(const std::string& personasDir, const std::string& personaName);

    MemoryDB& memory_;
    PersonaLoader& persona_;
    EmotionState* emotionState_ = nullptr;
    RelationState* relationState_ = nullptr;
    ConversationOrchestrator* orchestrator_ = nullptr;
    bool initialized_ = false;
};

}  // namespace brain
