#include "logger/logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>
#include <mutex>

/// 全局日志输出互斥锁
static std::mutex g_logMutex;

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

Logger::Logger() = default;

void Logger::setLevel(LogLevel level) {
    level_ = level;
}

LogLevel Logger::getLevel() const {
    return level_;
}

void Logger::debug(const std::string& tag, const std::string& msg) {
    log(LogLevel::DEBUG, tag, msg);
}

void Logger::info(const std::string& tag, const std::string& msg) {
    log(LogLevel::INFO, tag, msg);
}

void Logger::warn(const std::string& tag, const std::string& msg) {
    log(LogLevel::WARN, tag, msg);
}

void Logger::error(const std::string& tag, const std::string& msg) {
    log(LogLevel::ERROR, tag, msg);
}

void Logger::debug(const std::string& msg) {
    log(LogLevel::DEBUG, "", msg);
}

void Logger::info(const std::string& msg) {
    log(LogLevel::INFO, "", msg);
}

void Logger::warn(const std::string& msg) {
    log(LogLevel::WARN, "", msg);
}

void Logger::error(const std::string& msg) {
    log(LogLevel::ERROR, "", msg);
}

void Logger::log(LogLevel level, const std::string& tag, const std::string& msg) {
    if (level < level_) return;

    std::ostringstream oss;
    oss << "[" << getCurrentTime() << "] "
        << "[" << levelToString(level) << "]";

    if (!tag.empty()) {
        oss << " [" << tag << "]";
    }

    oss << " " << msg;

    std::lock_guard<std::mutex> lock(g_logMutex);
    if (level == LogLevel::ERROR) {
        std::cerr << oss.str() << std::endl;
    } else {
        std::cout << oss.str() << std::endl;
    }
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::ERROR: return "ERROR";
        default: return "     ";
    }
}

std::string Logger::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    struct tm tm_buf;
    localtime_r(&time_t, &tm_buf);

    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}
