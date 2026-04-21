#pragma once

struct RuntimeContext;

class FullModeRunner {
public:
    explicit FullModeRunner(RuntimeContext& context);
    void run();

private:
    RuntimeContext& context_;
};
