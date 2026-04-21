#include "app/mode_runner_chat.h"
#include "app/runtime_context.h"
#include "brain/brain_controller.h"
#include "brain/relation_state.h"
#include "brain/session.h"
#include <iostream>

ChatModeRunner::ChatModeRunner(RuntimeContext& context)
    : context_(context) {}

void ChatModeRunner::run() {
    std::cout << "=============================" << std::endl;
    std::cout << "  AI 桌面宠物 - 对话模式" << std::endl;
    std::cout << "  输入 'quit' 或 'exit' 退出" << std::endl;
    std::cout << "=============================" << std::endl;
    std::cout << std::endl;

    context_.brainController.setActiveMode("chat");
    context_.initializeSession();
    context_.brainController.setPersonaName(context_.session.personaName());
    context_.brainController.init(context_.eventBus, {});

    std::string input;
    while (true) {
        std::cout << "[" << context_.session.relationState().currentAffinityLevel() << " " << context_.session.relationState().currentAffinity() << "] 你: ";
        std::getline(std::cin, input);

        if (input == "quit" || input == "exit") {
            std::cout << "……再见。" << std::endl;
            break;
        }
        if (input.empty()) {
            continue;
        }

        std::cout << "（思考中...）" << std::endl;
        const brain::SessionTurnResult result = context_.brainController.processUserTextSync(input);
        if (result.success) {
            std::cout << "TA: " << result.reply << std::endl;
        } else if (!result.accepted) {
            std::cout << "TA: ……（我还在想上一句，你先等等）" << std::endl;
        } else {
            std::cout << "TA: ……（网络有点问题，再试一次？）" << std::endl;
        }

        std::cout << std::endl;
    }
}
