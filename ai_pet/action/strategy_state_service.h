#pragma once

#include "action/strategy_state.h"

namespace action {

class StrategyStateService {
public:
    StrategyState& state() { return state_; }
    const StrategyState& state() const { return state_; }
    void reset();

private:
    StrategyState state_;
};

}  // namespace action
