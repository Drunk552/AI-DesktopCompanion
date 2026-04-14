/**
 * @file config/config.h
 * @brief 配置管理系统
 * 
 * 提供统一的配置文件管理和加载功能，支持 JSON 格式配置。
 * 单例模式设计，全局共享配置状态。
 */

#pragma once

#include <string>
#include <memory>

/**
 * @brief 应用程序配置结构体
 * @details 包含所有可配置的参数，支持从 JSON 文件加载或使用默认值
 */
struct AppConfig {
    std::string ollama_url = "http://172.31.144.1:11434";  ///< Ollama API 服务器地址
    std::string model_name = "gemma4:e4b";                  ///< AI 模型名称
    
    std::string camera_sdp_path = "/tmp/stream.sdp";       ///< 摄像头 SDP 文件路径
    int camera_rtp_port = 5004;                            ///< RTP 视频流端口号
    
    std::string database_path = "data/memory.db";         ///< SQLite 数据库文件路径
    
    std::string face_model_path = "models/haarcascade_frontalface_default.xml";  ///< 人脸检测模型路径
    
    std::string personas_dir = "personas";                  ///< 角色文件目录
    
    int ui_window_width = 480;                             ///< UI 窗口宽度
    int ui_window_height = 320;                            ///< UI 窗口高度
    
    int face_detect_interval = 5;                          ///< 人脸检测间隔（每N帧检测一次）
    int emotion_stable_threshold = 3;                      ///< 情绪稳定判定阈值
};

/**
 * @class ConfigManager
 * @brief 配置管理器（单例模式）
 * @details 负责配置文件的加载、保存和访问
 */
class ConfigManager {
public:
    /**
     * @brief 获取配置管理器单例实例
     * @return ConfigManager& 配置管理器引用
     */
    static ConfigManager& instance();
    
    /**
     * @brief 加载配置文件
     * @param configPath 配置文件路径，默认为 "config.json"
     * @return bool 加载是否成功
     * @note 如果文件不存在，使用默认值并返回 true
     */
    bool load(const std::string& configPath = "config.json");
    
    /**
     * @brief 保存当前配置到文件
     * @param configPath 配置文件路径，为空则使用上次加载的路径
     * @return bool 保存是否成功
     */
    bool save(const std::string& configPath = "config.json") const;
    
    /**
     * @brief 获取配置引用（只读）
     * @return const AppConfig& 配置常量引用
     */
    const AppConfig& get() const { return config_; }
    
    /**
     * @brief 获取配置引用（可写）
     * @return AppConfig& 配置引用
     */
    AppConfig& get() { return config_; }
    
    /**
     * @brief 获取当前配置文件路径
     * @return std::string 配置文件路径
     */
    std::string getConfigPath() const { return configPath_; }
    
    /**
     * @brief 检查是否从文件加载了配置
     * @return bool 是否存在有效配置文件
     */
    bool hasConfigFile() const { return hasConfigFile_; }

private:
    ConfigManager() = default;                              ///< 私有构造函数（单例模式）
    ConfigManager(const ConfigManager&) = delete;          ///< 禁用拷贝构造
    ConfigManager& operator=(const ConfigManager&) = delete;///< 禁用赋值运算
    
    /**
     * @brief 解析 JSON 格式配置字符串
     * @param jsonStr JSON 字符串
     * @return bool 解析是否成功
     */
    bool parseJson(const std::string& jsonStr);
    
    /**
     * @brief 将当前配置转换为 JSON 字符串
     * @return std::string JSON 格式字符串
     */
    std::string toJson() const;
    
    AppConfig config_;          ///< 应用程序配置数据
    std::string configPath_;    ///< 配置文件路径
    bool hasConfigFile_ = false;///< 是否存在有效配置文件
};
