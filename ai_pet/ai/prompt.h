#pragma once

#include "ai/affinity.h"
#include "ai/persona_loader.h"
#include <string>

struct PromptContext {
    std::string userInput;
    std::string emotion = "平静";
    std::string context;
    int affinity = 30;
    std::string emotionTrend;
    const PersonaData* persona = nullptr;
};

std::string buildPrompt(const PromptContext& ctx);
