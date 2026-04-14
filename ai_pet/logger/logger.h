/**
 * @file logger/logger.h
 * @brief 日志系统
 * 
 * 提供统一格式的日志输出功能，支持多级别日志。
 * 单例模式设计，线程安全。
 * 
 * @note 默认输出到 stdout，ERROR 级别输出到 stderr
 */

#pragma once

#include <string>
#include <memory>
#include <atomic>

/**
 * @brief 日志级别枚举
 * @details 按严重程度从低到高排列
 */
enum class LogLevel {
    DEBUG = 0,  ///< 调试信息
    INFO = 1,   ///< 一般信息
    WARN = 2,   ///< 警告信息
    ERROR = 3,  ///< 错误信息
    NONE = 4    ///< 关闭日志
};

/**
 * @class Logger
 * @brief 日志记录器（单例模式）
 * @details 提供格式化的日志输出，支持不同级别和模块标签
 */
class Logger {
public:
    /**
     * @brief 获取日志记录器单例实例
     * @return Logger& 日志记录器引用
     */
    static Logger& instance();
    
    /**
     * @brief 设置日志级别
     * @param level 日志级别
     * @note 只会输出设置级别及以上的日志
     */
    void setLevel(LogLevel level);
    
    /**
     * @brief 获取当前日志级别
     * @return LogLevel 当前日志级别
     */
    LogLevel getLevel() const;
    
    /**
     * @brief 输出调试日志
     * @param tag 模块标签（如 "Brain", "Camera"）
     * @param msg 日志消息
     */
    void debug(const std::string& tag, const std::string& msg);
    
    /**
     * @brief 输出信息日志
     * @param tag 模块标签
     * @param msg 日志消息
     */
    void info(const std::string& tag, const std::string& msg);
    
    /**
     * @brief 输出警告日志
     * @param tag 模块标签
     * @param msg 日志消息
     */
    void warn(const std::string& tag, const std::string& msg);
    
    /**
     * @brief 输出错误日志
     * @param tag 模块标签
     * @param msg 日志消息
     */
    void error(const std::string& tag, const std::string& msg);
    
    /**
     * @brief 输出调试日志（无标签）
     * @param msg 日志消息
     */
    void debug(const std::string& msg);
    
    /**
     * @brief 输出信息日志（无标签）
     * @param msg 日志消息
     */
    void info(const std::string& msg);
    
    /**
     * @brief 输出警告日志（无标签）
     * @param msg 日志消息
     */
    void warn(const std::string& msg);
    
    /**
     * @brief 输出错误日志（无标签）
     * @param msg 日志消息
     */
    void error(const std::string& msg);

private:
    Logger();                                              ///< 私有构造函数
    Logger(const Logger&) = delete;                        ///< 禁用拷贝构造
    Logger& operator=(const Logger&) = delete;            ///< 禁用赋值运算
    
    /**
     * @brief 内部日志输出函数
     * @param level 日志级别
     * @param tag 模块标签
     * @param msg 日志消息
     */
    void log(LogLevel level, const std::string& tag, const std::string& msg);
    
    /**
     * @brief 将日志级别转换为字符串
     * @param level 日志级别
     * @return std::string 级别名称
     */
    std::string levelToString(LogLevel level);
    
    /**
     * @brief 获取当前时间戳字符串
     * @return std::string 格式: HH:MM:SS.mmm
     */
    std::string getCurrentTime();
    
    std::atomic<LogLevel> level_{LogLevel::INFO};  ///< 当前日志级别（线程安全）
};

/**
 * @brief DEBUG 级别日志宏
 * @param tag 模块标签
 * @param msg 日志消息
 */
#define LOGD(tag, msg) Logger::instance().debug(tag, msg)

/**
 * @brief INFO 级别日志宏
 * @param tag 模块标签
 * @param msg 日志消息
 */
#define LOGI(tag, msg) Logger::instance().info(tag, msg)

/**
 * @brief WARN 级别日志宏
 * @param tag 模块标签
 * @param msg 日志消息
 */
#define LOGW(tag, msg) Logger::instance().warn(tag, msg)

/**
 * @brief ERROR 级别日志宏
 * @param tag 模块标签
 * @param msg 日志消息
 */
#define LOGE(tag, msg) Logger::instance().error(tag, msg)
