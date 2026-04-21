#pragma once

#include <string>

class AppEventBus;

namespace action {

class PetActionController {
public:
    explicit PetActionController(AppEventBus& eventBus);

    void updateEmotion(const std::string& emotion);
    void updateAffection(int affinity);
    void updateRelationship(const std::string& relationship);

private:
    AppEventBus& eventBus_;
};

}  // namespace action
