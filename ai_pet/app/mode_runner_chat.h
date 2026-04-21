#pragma once

struct RuntimeContext;

class ChatModeRunner {
public:
    explicit ChatModeRunner(RuntimeContext& context);
    void run();

private:
    RuntimeContext& context_;
};
