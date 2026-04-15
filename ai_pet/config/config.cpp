#include "config.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

ConfigManager& ConfigManager::instance() {
    static ConfigManager manager;
    return manager;
}

bool ConfigManager::load(const std::string& configPath) {
    configPath_ = configPath;

    std::ifstream file(configPath);
    if (!file.is_open()) {
        std::cout << "[Config] 未找到配置文件，使用默认配置" << std::endl;
        hasConfigFile_ = false;
        return true;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    file.close();

    hasConfigFile_ = parseJson(ss.str());
    if (hasConfigFile_) {
        std::cout << "[Config] 已加载配置文件: " << configPath << std::endl;
    } else {
        std::cout << "[Config] 配置文件解析失败，使用默认配置" << std::endl;
    }
    return true;
}

bool ConfigManager::save(const std::string& configPath) const {
    std::string path = configPath.empty() ? configPath_ : configPath;
    if (path.empty()) {
        std::cerr << "[Config] 未指定配置文件路径" << std::endl;
        return false;
    }

    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "[Config] 无法创建配置文件: " << path << std::endl;
        return false;
    }

    file << toJson();
    file.close();
    std::cout << "[Config] 配置文件已保存: " << path << std::endl;
    return true;
}

bool ConfigManager::parseJson(const std::string& jsonStr) {
    try {
        auto j = json::parse(jsonStr);

        if (j.contains("ollama_url"))  config_.ollama_url = j["ollama_url"].get<std::string>();
        if (j.contains("model_name"))  config_.model_name = j["model_name"].get<std::string>();
        if (j.contains("camera_sdp_path"))  config_.camera_sdp_path = j["camera_sdp_path"].get<std::string>();
        if (j.contains("camera_rtp_port"))  config_.camera_rtp_port = j["camera_rtp_port"].get<int>();
        if (j.contains("database_path"))  config_.database_path = j["database_path"].get<std::string>();
        if (j.contains("face_model_path"))  config_.face_model_path = j["face_model_path"].get<std::string>();
        if (j.contains("personas_dir"))  config_.personas_dir = j["personas_dir"].get<std::string>();
        if (j.contains("ui_window_width"))  config_.ui_window_width = j["ui_window_width"].get<int>();
        if (j.contains("ui_window_height"))  config_.ui_window_height = j["ui_window_height"].get<int>();
        if (j.contains("face_detect_interval"))  config_.face_detect_interval = j["face_detect_interval"].get<int>();
        if (j.contains("emotion_stable_threshold"))  config_.emotion_stable_threshold = j["emotion_stable_threshold"].get<int>();

        return true;
    } catch (const json::exception& e) {
        std::cerr << "[Config] JSON 解析错误: " << e.what() << std::endl;
        return false;
    }
}

std::string ConfigManager::toJson() const {
    json j = {
        {"ollama_url", config_.ollama_url},
        {"model_name", config_.model_name},
        {"camera_sdp_path", config_.camera_sdp_path},
        {"camera_rtp_port", config_.camera_rtp_port},
        {"database_path", config_.database_path},
        {"face_model_path", config_.face_model_path},
        {"personas_dir", config_.personas_dir},
        {"ui_window_width", config_.ui_window_width},
        {"ui_window_height", config_.ui_window_height},
        {"face_detect_interval", config_.face_detect_interval},
        {"emotion_stable_threshold", config_.emotion_stable_threshold}
    };
    return j.dump(2);
}
