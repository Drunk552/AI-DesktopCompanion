/**
 * @file ai/gemma.h
 * @brief Gemma AI 接口模块
 * 
 * 封装与 Ollama 本地 AI 模型的交互，
 * 支持同步和异步调用，提供超时和取消机制。
 */

#pragma once

#include <string>
#include <future>
#include <atomic>
#include <functional>

/**
 * @class GemmaAI
 * @brief Gemma AI 对话接口
 * @details 通过 HTTP API 与 Ollama 服务通信，支持 Gemma 等本地模型
 */
class GemmaAI {
public:
    /**
     * @brief 构造函数
     * @param ollamaUrl Ollama 服务地址，为空则自动检测
     * @param model 模型名称，默认为 "gemma4:e4b"
     */
    explicit GemmaAI(const std::string& ollamaUrl = "",
                     const std::string& model = "gemma4:e4b");
    
    /**
     * @brief 析构函数
     * @note 会自动取消正在进行的请求
     */
    ~GemmaAI();
    
    /**
     * @brief 同步发送对话请求
     * @param prompt 输入的提示词
     * @return std::string AI 生成的回复，空字符串表示失败
     */
    std::string chat(const std::string& prompt);
    
    /**
     * @brief 异步发送对话请求
     * @param prompt 输入的提示词
     * @return std::future<std::string> 包含回复的 future
     */
    std::future<std::string> chatAsync(const std::string& prompt);
    
    /**
     * @brief 取消正在进行的请求
     */
    void cancel();
    
    /**
     * @brief 检查是否正在处理请求
     * @return bool 是否正在运行
     */
    bool isRunning() const { return running_.load(); }
    
    /**
     * @brief 设置使用的模型
     * @param model 模型名称
     */
    void setModel(const std::string& model);
    
    /**
     * @brief 设置请求超时时间
     * @param seconds 超时秒数
     */
    void setTimeout(int seconds);

private:
    /**
     * @brief 实际执行请求的内部函数
     * @param prompt 提示词
     * @return std::string 回复内容
     */
    std::string chatImpl(const std::string& prompt);
    
    std::string ollamaUrl_;       ///< Ollama 服务地址
    std::string model_;           ///< 模型名称
    int timeoutSeconds_ = 300;   ///< 超时秒数
    std::atomic<bool> running_{false};    ///< 是否正在运行
    std::atomic<bool> cancelled_{false}; ///< 是否已取消
    
    /**
     * @brief libcurl 写入回调函数
     */
    static size_t writeCallback(char* ptr, size_t size, size_t nmemb, std::string* data);
};
