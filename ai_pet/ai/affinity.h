#pragma once

#include <string>

enum class AffinityLevel {
    Stranger,
    Distant,
    Familiar,
    Close,
    Intimate
};

AffinityLevel getAffinityLevel(int affinity);
std::string getAffinityLevelName(AffinityLevel level);
int calculateAffinityDelta(const std::string& userEmotion, const std::string& userText);
