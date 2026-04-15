#pragma once

#include "ai/affinity.h"
#include <string>

struct FallbackPersona {
    std::string role;
    std::string tone;
    std::string trait;
    std::string style;
};

FallbackPersona evolveFallbackPersona(int affinity, const std::string& emotionTrend);
std::string getFallbackStrategy(const std::string& emotion, AffinityLevel level);
