#include "brain/emotion_state.h"

namespace brain {

void EmotionState::updateDetectedEmotion(const std::string& emotion) {
    if (emotion.empty()) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    currentEmotion_ = emotion;
}

std::string EmotionState::currentEmotion() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return currentEmotion_;
}

}  // namespace brain
