/**
 * @file main.cpp
 * @brief AI 桌面宠物主程序入口
 * 
 * 支持多种运行模式：
 * - UI 模式：LVGL 图形界面
 * - 对话模式：终端纯文字交互
 * - 摄像头模式：测试摄像头和人脸检测
 * - 完整模式：终端+摄像头
 * 
 * @example
 * ./ai_pet                  // 默认 UI 模式
 * ./ai_pet --chat           // 终端对话模式
 * ./ai_pet --camera         // 摄像头测试
 * ./ai_pet --full           // 完整模式
 * ./ai_pet --persona xiao   // 指定角色
 * ./ai_pet --config my.json  // 指定配置
 */

#include "controller/brain.h"
#include "ai/persona_loader.h"
#include "config/config.h"
#include <iostream>
#include <string>
#include <cstring>

/**
 * @brief 打印程序使用帮助
 * @param prog 程序名称
 */
void printUsage(const char* prog) {
    std::cout << "用法: " << prog << " [选项]" << std::endl;
    std::cout << "  --ui              UI 模式 - LVGL 窗口（默认）" << std::endl;
    std::cout << "  --chat            纯对话模式" << std::endl;
    std::cout << "  --camera          摄像头测试模式" << std::endl;
    std::cout << "  --full            完整模式（终端+摄像头）" << std::endl;
    std::cout << "  --persona <name>  指定加载的角色名称" << std::endl;
    std::cout << "  --list-personas   列出可用角色" << std::endl;
    std::cout << "  --config <path>   指定配置文件路径（默认: config.json）" << std::endl;
    std::cout << "  --help            显示帮助" << std::endl;
}

/**
 * @brief 主函数
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return int 退出码
 */
int main(int argc, char* argv[]) {
    std::string mode = "ui";      ///< 运行模式
    std::string personaName;      ///< 指定的角色名称
    std::string configPath = "config.json";  ///< 配置文件路径
    
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--ui") == 0) {
            mode = "ui";
        } else if (std::strcmp(argv[i], "--chat") == 0) {
            mode = "chat";
        } else if (std::strcmp(argv[i], "--camera") == 0) {
            mode = "camera";
        } else if (std::strcmp(argv[i], "--full") == 0) {
            mode = "full";
        } else if (std::strcmp(argv[i], "--persona") == 0) {
            if (i + 1 < argc) {
                personaName = argv[++i];
            } else {
                std::cerr << "--persona 需要指定角色名称" << std::endl;
                return 1;
            }
        } else if (std::strcmp(argv[i], "--config") == 0) {
            if (i + 1 < argc) {
                configPath = argv[++i];
            } else {
                std::cerr << "--config 需要指定配置文件路径" << std::endl;
                return 1;
            }
        } else if (std::strcmp(argv[i], "--list-personas") == 0) {
            auto personas = PersonaLoader::listPersonas("personas");
            if (personas.empty()) {
                std::cout << "未找到可用角色。请将角色文件放到 personas/ 目录下。" << std::endl;
            } else {
                std::cout << "可用角色：" << std::endl;
                for (const auto& p : personas) {
                    std::cout << "  - " << p << std::endl;
                }
            }
            return 0;
        } else if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            printUsage(argv[0]);
            return 0;
        } else {
            std::cerr << "未知参数: " << argv[i] << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    ConfigManager::instance().load(configPath);
    Brain brain(configPath);
    
    if (!personaName.empty()) {
        brain.setPersonaName(personaName);
    }
    
    if (mode == "ui") {
        brain.runUIMode();
    } else if (mode == "chat") {
        brain.runChatMode();
    } else if (mode == "camera") {
        brain.runCameraMode();
    } else if (mode == "full") {
        brain.runFullMode();
    }
    
    return 0;
}
