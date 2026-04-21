#include "brain/session.h"
#include "brain/conversation_orchestrator.h"
#include "brain/emotion_state.h"
#include "brain/relation_state.h"
#include "shared/logger/logger.h"

#include <memory>

namespace brain {

namespace {

std::unique_ptr<EmotionState> create_emotion_state() {
    return std::make_unique<EmotionState>();
}

std::unique_ptr<RelationState> create_relation_state(MemoryDB& memory) {
    return std::make_unique<RelationState>(memory);
}

std::unique_ptr<ConversationOrchestrator> create_orchestrator(
    GemmaAI& ai,
    MemoryDB& memory,
    PersonaLoader& persona,
    EmotionState& emotionState,
    RelationState& relationState
) {
    return std::make_unique<ConversationOrchestrator>(ai, memory, persona, emotionState, relationState);
}

}  // namespace

Session::Session(GemmaAI& ai, MemoryDB& memory, PersonaLoader& persona)
    : memory_(memory)
    , persona_(persona) {
    auto emotionState = create_emotion_state();
    auto relationState = create_relation_state(memory_);
    auto orchestrator = create_orchestrator(ai, memory_, persona_, *emotionState, *relationState);

    emotionState_ = emotionState.release();
    relationState_ = relationState.release();
    orchestrator_ = orchestrator.release();
}

Session::~Session() {
    orchestrator_->cancelPending();
    delete orchestrator_;
    delete relationState_;
    delete emotionState_;
}

void Session::initialize(const std::string& personasDir, const std::string& personaName) {
    if (!initialized_) {
        if (memory_.open()) {
            LOGI("Session", "记忆系统已启动");
        }
        initialized_ = true;
    }

    loadPersona(personasDir, personaName);

    const int affinity = relationState_->currentAffinity();
    LOGI(
        "Session",
        "好感度: " + std::to_string(affinity) + "/100 (" + relationState_->currentAffinityLevel() + ")"
    );
}

std::string Session::personaName() const {
    return persona_.isLoaded() ? persona_.getData().name : std::string();
}

ConversationOrchestrator& Session::orchestrator() {
    return *orchestrator_;
}

const ConversationOrchestrator& Session::orchestrator() const {
    return *orchestrator_;
}

EmotionState& Session::emotionState() {
    return *emotionState_;
}

const EmotionState& Session::emotionState() const {
    return *emotionState_;
}

RelationState& Session::relationState() {
    return *relationState_;
}

const RelationState& Session::relationState() const {
    return *relationState_;
}

void Session::loadPersona(const std::string& personasDir, const std::string& personaName) {
    if (!personaName.empty()) {
        const std::string dir = personasDir + "/" + personaName;
        if (persona_.load(dir)) {
            LOGI("Session", "已加载角色: " + persona_.getData().name);
        } else {
            LOGE("Session", "指定角色 '" + personaName + "' 加载失败");
        }
        return;
    }

    if (persona_.autoLoad(personasDir)) {
        LOGI("Session", "已加载角色: " + persona_.getData().name);
    } else {
        LOGW("Session", "未找到角色文件，使用默认人格");
    }
}

}  // namespace brain
