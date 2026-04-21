#pragma once

struct RuntimeContext;

class UIModeRunner {
public:
    explicit UIModeRunner(RuntimeContext& context);
    void run();

private:
    RuntimeContext& context_;
};
