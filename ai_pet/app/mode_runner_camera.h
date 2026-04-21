#pragma once

struct RuntimeContext;

class CameraModeRunner {
public:
    explicit CameraModeRunner(RuntimeContext& context);
    void run();

private:
    RuntimeContext& context_;
};
