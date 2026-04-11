#include "ai/gemma.h"
#include <iostream>
#include <fstream>
#include <array>
#include <cstdio>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// 自动获取 WSL2 网关 IP（即 Windows 主机 IP）
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
    // 去除末尾换行
    while (!ip.empty() && (ip.back() == '\n' || ip.back() == '\r'))
        ip.pop_back();
    return ip;
}

GemmaAI::GemmaAI(const std::string& ollamaUrl, const std::string& model)
    : model_(model) {
    if (ollamaUrl.empty()) {
        // 自动检测 Windows 主机 IP
        std::string hostIP = getWindowsHostIP();
        if (hostIP.empty()) hostIP = "localhost";
        ollamaUrl_ = "http://" + hostIP + ":11434";
        std::cout << "[GemmaAI] 自动检测 Ollama 地址: " << ollamaUrl_ << std::endl;
    } else {
        ollamaUrl_ = ollamaUrl;
    }
}

void GemmaAI::setModel(const std::string& model) {
    model_ = model;
}

size_t GemmaAI::writeCallback(char* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

std::string GemmaAI::chat(const std::string& prompt) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "[GemmaAI] 无法初始化 CURL" << std::endl;
        return "";
    }

    // 构造请求 JSON
    json requestBody = {
        {"model", model_},
        {"prompt", prompt},
        {"stream", false}
    };
    std::string requestStr = requestBody.dump();

    // 设置 URL
    std::string url = ollamaUrl_ + "/api/generate";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // 设置 POST 数据
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestStr.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, requestStr.size());

    // 设置 Header
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // 设置超时（大模型推理可能较慢）
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);

    // 设置回调
    std::string response;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // 执行请求
    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "[GemmaAI] 请求失败: " << curl_easy_strerror(res) << std::endl;
        return "";
    }

    // 解析响应
    try {
        json respJson = json::parse(response);
        if (respJson.contains("response")) {
            return respJson["response"].get<std::string>();
        } else if (respJson.contains("error")) {
            std::cerr << "[GemmaAI] API 错误: " << respJson["error"] << std::endl;
        }
    } catch (const json::exception& e) {
        std::cerr << "[GemmaAI] JSON 解析失败: " << e.what() << std::endl;
    }

    return "";
}
