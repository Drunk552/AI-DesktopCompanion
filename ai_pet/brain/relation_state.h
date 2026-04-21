#pragma once

#include "intelligence/memory/memory_db.h"
#include <string>

namespace brain {

struct RelationSnapshot {
    int affinity = 30;
    std::string affinityLevel = "疏离";
};

class RelationState {
public:
    explicit RelationState(MemoryDB& memory);

    int currentAffinity() const;
    std::string currentAffinityLevel() const;
    std::string recentEmotionTrend() const;
    RelationSnapshot snapshot() const;
    void updateAfterConversation(const std::string& userEmotion, const std::string& userText);

private:
    MemoryDB& memory_;
};

}  // namespace brain
