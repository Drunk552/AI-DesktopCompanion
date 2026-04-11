#pragma once

#include <string>
#include <memory>

struct AppConfig {
    std::string ollama_url = "http://172.31.144.1:11434";
    std::string model_name = "gemma4:e4b";
    
    std::string camera_sdp_path = "/tmp/stream.sdp";
    int camera_rtp_port = 5004;
    
    std::string database_path = "data/memory.db";
    
    std::string face_model_path = "models/haarcascade_frontalface_default.xml";
    
    std::string personas_dir = "personas";
    
    int ui_window_width = 480;
    int ui_window_height = 320;
    
    int face_detect_interval = 5;
    int emotion_stable_threshold = 3;
};

class ConfigManager {
public:
    static ConfigManager& instance();
    
    bool load(const std::string& configPath = "config.json");
    bool save(const std::string& configPath = "config.json") const;
    
    const AppConfig& get() const { return config_; }
    AppConfig& get() { return config_; }
    
    std::string getConfigPath() const { return configPath_; }
    bool hasConfigFile() const { return hasConfigFile_; }

private:
    ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    bool parseJson(const std::string& jsonStr);
    std::string toJson() const;
    
    AppConfig config_;
    std::string configPath_;
    bool hasConfigFile_ = false;
};
