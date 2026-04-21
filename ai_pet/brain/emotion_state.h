#pragma once

#include <mutex>
#include <string>

namespace brain {

class EmotionState {
public:
    void updateDetectedEmotion(const std::string& emotion);
    std::string currentEmotion() const;

private:
    mutable std::mutex mutex_;
    std::string currentEmotion_ = "平静";
};

}  // namespace brain
