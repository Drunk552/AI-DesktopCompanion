/**
 * @file memory/sqlite.h
 * @brief 记忆数据库模块
 * 
 * 使用 SQLite 存储对话历史、情绪记录和关系数据。
 * 提供长期记忆功能，让 AI 能够记住之前的对话。
 */

#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <sqlite3.h>

/**
 * @struct ChatRecord
 * @brief 对话记录结构
 */
struct ChatRecord {
    int id;                  ///< 记录 ID
    std::string userText;    ///< 用户输入
    std::string aiReply;     ///< AI 回复
    std::string emotion;      ///< 当时的情绪
    std::string timestamp;   ///< 时间戳
};

/**
 * @class MemoryDB
 * @brief 记忆数据库管理器
 * @details 封装 SQLite 操作，提供对话存储和关系系统接口
 */
class MemoryDB {
public:
    /**
     * @brief 构造函数
     * @param dbPath 数据库文件路径
     */
    explicit MemoryDB(const std::string& dbPath = "data/memory.db");
    
    /**
     * @brief 析构函数，自动关闭数据库
     */
    ~MemoryDB();
    
    /**
     * @brief 打开数据库
     * @return bool 是否成功
     * @note 会自动创建必要的表
     */
    bool open();
    
    /**
     * @brief 关闭数据库连接
     */
    void close();
    
    /**
     * @brief 保存一轮对话
     * @param userText 用户输入
     * @param aiReply AI 回复
     * @param emotion 当时的情绪
     * @return bool 是否保存成功
     */
    bool saveChat(const std::string& userText, const std::string& aiReply, const std::string& emotion);
    
    /**
     * @brief 获取最近的对话记录
     * @param n 获取数量
     * @return std::vector<ChatRecord> 对话记录列表
     */
    std::vector<ChatRecord> getRecentChats(int n = 5);
    
    /**
     * @brief 获取上下文摘要
     * @param n 参考的对话轮数
     * @return std::string 格式化的上下文字符串
     */
    std::string getContext(int n = 5);
    
    // ==================== 关系系统 ====================
    
    /**
     * @brief 获取当前好感度
     * @return int 好感度值（0-100）
     */
    int getAffinity();
    
    /**
     * @brief 更新好感度
     * @param delta 变化值
     * @note 会自动钳位到 0-100 范围
     */
    void updateAffinity(int delta);
    
    /**
     * @brief 记录用户情绪
     * @param emotion 情绪类别
     */
    void recordEmotion(const std::string& emotion);
    
    /**
     * @brief 获取情绪趋势摘要
     * @param n 统计的轮数
     * @return std::string 趋势描述
     */
    std::string getEmotionTrend(int n = 20);
    
    /**
     * @brief 获取总对话轮数
     * @return int 对话总数
     */
    int getTotalChats();
    
    /**
     * @brief 增加对话计数
     */
    void incrementChatCount();

private:
    /**
     * @brief 确保关系表有初始数据
     */
    void ensureRelationshipRow();
    
    std::string dbPath_;  ///< 数据库文件路径
    sqlite3* db_ = nullptr;  ///< SQLite 数据库句柄
    mutable std::mutex dbMutex_;  ///< 数据库操作互斥锁
};
