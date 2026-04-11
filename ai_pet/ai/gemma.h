#pragma once

#include <string>

class GemmaAI {
public:
    // ollamaUrl: Ollama API 地址
    // model: 模型名称
    // 默认通过 WSL2 网关访问 Windows 上的 Ollama
    explicit GemmaAI(const std::string& ollamaUrl = "",
                     const std::string& model = "gemma4:e4b");
    ~GemmaAI() = default;

    // 发送 prompt，返回 AI 生成的文本
    std::string chat(const std::string& prompt);

    // 设置模型名称
    void setModel(const std::string& model);

private:
    std::string ollamaUrl_;
    std::string model_;

    // libcurl 回调
    static size_t writeCallback(char* ptr, size_t size, size_t nmemb, std::string* data);
};
