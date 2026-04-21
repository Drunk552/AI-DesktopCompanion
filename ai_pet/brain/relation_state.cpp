#include "brain/relation_state.h"
#include "intelligence/relation/affinity.h"

namespace brain {

RelationState::RelationState(MemoryDB& memory)
    : memory_(memory) {}

int RelationState::currentAffinity() const {
    return memory_.getAffinity();
}

std::string RelationState::currentAffinityLevel() const {
    return getAffinityLevelName(getAffinityLevel(currentAffinity()));
}

std::string RelationState::recentEmotionTrend() const {
    return memory_.getEmotionTrend();
}

RelationSnapshot RelationState::snapshot() const {
    return {currentAffinity(), currentAffinityLevel()};
}

void RelationState::updateAfterConversation(const std::string& userEmotion, const std::string& userText) {
    memory_.updateAffinity(calculateAffinityDelta(userEmotion, userText));
}

}  // namespace brain
