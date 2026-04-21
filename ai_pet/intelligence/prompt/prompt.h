#pragma once

#include "intelligence/persona/persona_loader.h"
#include "intelligence/relation/affinity.h"
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
