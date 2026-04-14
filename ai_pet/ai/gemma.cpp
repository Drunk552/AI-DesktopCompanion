#include "ai/gemma.h"
#include "logger/logger.h"
#include <fstream>
#include <array>
#include <cstdio>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static std::string getWindowsHostIP() {
    std::string ip;
    std::array<char, 128> buffer;
    FILE* pipe = popen("ip route show default | awk '{print $3}'", "r");
    if (pipe) {
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            ip += buffer.data();
        }
        pclose(pipe);
    }
    while (!ip.empty() && (ip.back() == '\n' || ip.back() == '\r'))
        ip.pop_back();
    return ip;
}

GemmaAI::GemmaAI(const std::string& ollamaUrl, const std::string& model)
    : model_(model) {
    if (ollamaUrl.empty()) {
        std::string hostIP = getWindowsHostIP();
        if (hostIP.empty()) hostIP = "localhost";
        ollamaUrl_ = "http://" + hostIP + ":11434";
        LOGI("Gemma", "自动检测 Ollama 地址: " + ollamaUrl_);
    } else {
        ollamaUrl_ = ollamaUrl;
    }
}

GemmaAI::~GemmaAI() {
    cancel();
}

void GemmaAI::setModel(const std::string& model) {
    model_ = model;
}

void GemmaAI::setTimeout(int seconds) {
    timeoutSeconds_ = seconds;
}

void GemmaAI::cancel() {
    cancelled_.store(true);
}

size_t GemmaAI::writeCallback(char* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

std::string GemmaAI::chat(const std::string& prompt) {
    return chatImpl(prompt);
}

std::future<std::string> GemmaAI::chatAsync(const std::string& prompt) {
    return std::async(std::launch::async, [this, prompt]() {
        return chatImpl(prompt);
    });
}

std::string GemmaAI::chatImpl(const std::string& prompt) {
    running_.store(true);
    cancelled_.store(false);
    
    LOGD("Gemma", "开始调用 AI...");
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        LOGE("Gemma", "无法初始化 CURL");
        running_.store(false);
        return "";
    }

    json requestBody = {
        {"model", model_},
        {"prompt", prompt},
        {"stream", false}
    };
    std::string requestStr = requestBody.dump();

    std::string url = ollamaUrl_ + "/api/generate";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestStr.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, requestStr.size());

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_TIMEOUT, static_cast<long>(timeoutSeconds_));

    std::string response;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    running_.store(false);

    if (cancelled_.load()) {
        LOGW("Gemma", "请求已被取消");
        return "";
    }

    if (res != CURLE_OK) {
        LOGE("Gemma", "请求失败: " + std::string(curl_easy_strerror(res)));
        return "";
    }

    try {
        json respJson = json::parse(response);
        if (respJson.contains("response")) {
            std::string result = respJson["response"].get<std::string>();
            LOGD("Gemma", "AI 响应完成，长度: " + std::to_string(result.size()) + " 字符");
            return result;
        } else if (respJson.contains("error")) {
            LOGE("Gemma", "API 错误: " + respJson["error"].get<std::string>());
        }
    } catch (const json::exception& e) {
        LOGE("Gemma", "JSON 解析失败: " + std::string(e.what()));
    }

    return "";
}
