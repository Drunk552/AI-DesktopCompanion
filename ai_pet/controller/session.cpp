#include "controller/session.h"
#include "ai/affinity.h"
#include "ai/prompt.h"
#include "logger/logger.h"
#include <chrono>

Session::Session(GemmaAI& ai, MemoryDB& memory, PersonaLoader& persona)
    : ai_(ai)
    , memory_(memory)
    , persona_(persona) {}

Session::~Session() {
    cancelPending();
}

void Session::initialize(const std::string& personasDir, const std::string& personaName) {
    if (!initialized_) {
        if (memory_.open()) {
            LOGI("Session", "记忆系统已启动");
        }
        initialized_ = true;
    }

    loadPersona(personasDir, personaName);

    const int affinity = memory_.getAffinity();
    LOGI(
        "Session",
        "好感度: " + std::to_string(affinity) + "/100 (" + getAffinityLevelName(getAffinityLevel(affinity)) + ")"
    );
}

SessionTurnResult Session::handleUserInput(const std::string& input, int timeoutMs) {
    return handleUserInputImpl(input, timeoutMs, true);
}

SessionTurnResult Session::handleReservedUserInput(const std::string& input, int timeoutMs) {
    return handleUserInputImpl(input, timeoutMs, false);
}

bool Session::tryBeginTurn() {
    return !aiRunning_.exchange(true);
}

SessionTurnResult Session::handleUserInputImpl(const std::string& input, int timeoutMs, bool reserveTurn) {
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
        result.emotion = currentEmotion();

        const int affinity = memory_.getAffinity();
        PromptContext pctx;
        pctx.userInput = input;
        pctx.emotion = result.emotion;
        pctx.context = memory_.getContext(5);
        pctx.affinity = affinity;
        pctx.emotionTrend = memory_.getEmotionTrend();
        pctx.persona = getPersonaPtr();
        const std::string prompt = buildPrompt(pctx);

        auto future = ai_.chatAsync(prompt);
        std::string reply;

        if (waitForAI(future, reply, timeoutMs)) {
            if (reply.empty()) {
                reply = "……";
            }

            memory_.saveChat(input, reply, result.emotion);
            memory_.recordEmotion(result.emotion);
            memory_.incrementChatCount();
            memory_.updateAffinity(calculateAffinityDelta(result.emotion, input));

            result.success = true;
            result.reply = reply;
        }
    } catch (const std::exception& e) {
        LOGE("Session", "处理消息异常: " + std::string(e.what()));
    } catch (...) {
        LOGE("Session", "处理消息发生未知异常");
    }

    result.affinity = memory_.getAffinity();
    result.affinityLevel = getAffinityLevelName(getAffinityLevel(result.affinity));
    aiRunning_.store(false);
    return result;
}

void Session::onEmotionDetected(const std::string& emotion) {
    if (emotion.empty()) {
        return;
    }

    std::lock_guard<std::mutex> lock(emotionMutex_);
    currentEmotion_ = emotion;
}

void Session::cancelPending() {
    cancelled_.store(true);
    ai_.cancel();
}

std::string Session::currentEmotion() const {
    std::lock_guard<std::mutex> lock(emotionMutex_);
    return currentEmotion_;
}

int Session::currentAffinity() const {
    return memory_.getAffinity();
}

std::string Session::currentAffinityLevel() const {
    return getAffinityLevelName(getAffinityLevel(currentAffinity()));
}

bool Session::waitForAI(std::future<std::string>& future, std::string& result, int timeoutMs) {
    const auto status = future.wait_for(std::chrono::milliseconds(timeoutMs));
    if (status == std::future_status::ready) {
        try {
            result = future.get();
            return true;
        } catch (const std::exception& e) {
            LOGE("Session", "AI 调用异常: " + std::string(e.what()));
            return false;
        }
    }

    if (status == std::future_status::timeout) {
        LOGW("Session", "AI 调用超时");
        cancelled_.store(true);
        ai_.cancel();
    }
    return false;
}

const PersonaData* Session::getPersonaPtr() const {
    return persona_.isLoaded() ? &persona_.getData() : nullptr;
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
