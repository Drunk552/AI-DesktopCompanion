#pragma once

#include "ai/gemma.h"
#include "ai/persona_loader.h"
#include "memory/sqlite.h"
#include <atomic>
#include <future>
#include <mutex>
#include <string>

struct SessionTurnResult {
    bool accepted = false;
    bool success = false;
    std::string reply;
    std::string emotion;
    int affinity = 30;
    std::string affinityLevel;
};

class Session {
public:
    Session(GemmaAI& ai, MemoryDB& memory, PersonaLoader& persona);
    ~Session();

    void initialize(const std::string& personasDir, const std::string& personaName = "");
    SessionTurnResult handleUserInput(const std::string& input, int timeoutMs = 120000);
    SessionTurnResult handleReservedUserInput(const std::string& input, int timeoutMs = 120000);
    void onEmotionDetected(const std::string& emotion);
    void cancelPending();
    bool tryBeginTurn();

    bool isBusy() const { return aiRunning_.load(); }
    std::string currentEmotion() const;
    int currentAffinity() const;
    std::string currentAffinityLevel() const;

private:
    SessionTurnResult handleUserInputImpl(const std::string& input, int timeoutMs, bool reserveTurn);
    bool waitForAI(std::future<std::string>& future, std::string& result, int timeoutMs);
    const PersonaData* getPersonaPtr() const;
    void loadPersona(const std::string& personasDir, const std::string& personaName);

    GemmaAI& ai_;
    MemoryDB& memory_;
    PersonaLoader& persona_;
    mutable std::mutex emotionMutex_;
    std::string currentEmotion_ = "平静";
    std::atomic<bool> aiRunning_{false};
    std::atomic<bool> cancelled_{false};
    bool initialized_ = false;
};
