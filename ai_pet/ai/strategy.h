#pragma once

#include "ai/personality.h"
#include <string>

// 根据用户情绪和好感度等级，返回对应的回应策略
std::string getStrategy(const std::string& emotion, AffinityLevel level);
