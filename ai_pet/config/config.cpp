#include "config.h"
#include <fstream>
#include <iostream>
#include <sstream>

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
    size_t pos = 0;
    
    auto skipWhitespace = [&]() {
        while (pos < jsonStr.size() && std::isspace(jsonStr[pos])) pos++;
    };
    
    auto expectChar = [&](char c) -> bool {
        skipWhitespace();
        if (pos >= jsonStr.size() || jsonStr[pos] != c) return false;
        pos++;
        return true;
    };
    
    auto parseString = [&]() -> std::string {
        skipWhitespace();
        if (pos >= jsonStr.size() || jsonStr[pos] != '"') return "";
        pos++;
        std::string result;
        while (pos < jsonStr.size() && jsonStr[pos] != '"') {
            if (jsonStr[pos] == '\\' && pos + 1 < jsonStr.size()) {
                pos++;
                switch (jsonStr[pos]) {
                    case 'n': result += '\n'; break;
                    case 't': result += '\t'; break;
                    case 'r': result += '\r'; break;
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    default: result += jsonStr[pos]; break;
                }
            } else {
                result += jsonStr[pos];
            }
            pos++;
        }
        if (pos < jsonStr.size()) pos++;
        return result;
    };
    
    auto parseNumber = [&]() -> int {
        skipWhitespace();
        size_t start = pos;
        if (pos < jsonStr.size() && (jsonStr[pos] == '-' || jsonStr[pos] == '+')) pos++;
        while (pos < jsonStr.size() && std::isdigit(jsonStr[pos])) pos++;
        return std::stoi(jsonStr.substr(start, pos - start));
    };
    
    auto parseKeyValue = [&](const std::string& key) -> bool {
        skipWhitespace();
        if (jsonStr.compare(pos, key.size(), key) != 0) return false;
        pos += key.size();
        skipWhitespace();
        if (pos >= jsonStr.size() || jsonStr[pos] != ':') return false;
        pos++;
        
        if (key == "\"ollama_url\"" || key == "\"model_name\"" ||
            key == "\"camera_sdp_path\"" || key == "\"database_path\"" ||
            key == "\"face_model_path\"" || key == "\"personas_dir\"") {
            std::string val = parseString();
            if (key == "\"ollama_url\"") config_.ollama_url = val;
            else if (key == "\"model_name\"") config_.model_name = val;
            else if (key == "\"camera_sdp_path\"") config_.camera_sdp_path = val;
            else if (key == "\"database_path\"") config_.database_path = val;
            else if (key == "\"face_model_path\"") config_.face_model_path = val;
            else if (key == "\"personas_dir\"") config_.personas_dir = val;
            return true;
        } else if (key == "\"camera_rtp_port\"" || key == "\"ui_window_width\"" ||
                   key == "\"ui_window_height\"" || key == "\"face_detect_interval\"" ||
                   key == "\"emotion_stable_threshold\"") {
            int val = parseNumber();
            if (key == "\"camera_rtp_port\"") config_.camera_rtp_port = val;
            else if (key == "\"ui_window_width\"") config_.ui_window_width = val;
            else if (key == "\"ui_window_height\"") config_.ui_window_height = val;
            else if (key == "\"face_detect_interval\"") config_.face_detect_interval = val;
            else if (key == "\"emotion_stable_threshold\"") config_.emotion_stable_threshold = val;
            return true;
        }
        return false;
    };
    
    skipWhitespace();
    if (!expectChar('{')) return false;
    
    while (true) {
        skipWhitespace();
        if (pos >= jsonStr.size()) return false;
        if (jsonStr[pos] == '}') break;
        if (jsonStr[pos] == ',') {
            pos++;
            continue;
        }
        if (jsonStr[pos] == '"') {
            std::string key = parseString();
            if (!key.empty()) {
                parseKeyValue("\"" + key + "\"");
            }
        } else {
            pos++;
        }
    }
    
    return true;
}

std::string ConfigManager::toJson() const {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"ollama_url\": \"" << config_.ollama_url << "\",\n";
    ss << "  \"model_name\": \"" << config_.model_name << "\",\n";
    ss << "  \"camera_sdp_path\": \"" << config_.camera_sdp_path << "\",\n";
    ss << "  \"camera_rtp_port\": " << config_.camera_rtp_port << ",\n";
    ss << "  \"database_path\": \"" << config_.database_path << "\",\n";
    ss << "  \"face_model_path\": \"" << config_.face_model_path << "\",\n";
    ss << "  \"personas_dir\": \"" << config_.personas_dir << "\",\n";
    ss << "  \"ui_window_width\": " << config_.ui_window_width << ",\n";
    ss << "  \"ui_window_height\": " << config_.ui_window_height << ",\n";
    ss << "  \"face_detect_interval\": " << config_.face_detect_interval << ",\n";
    ss << "  \"emotion_stable_threshold\": " << config_.emotion_stable_threshold << "\n";
    ss << "}\n";
    return ss.str();
}
