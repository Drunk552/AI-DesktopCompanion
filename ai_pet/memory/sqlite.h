#pragma once

#include <string>
#include <vector>
#include <sqlite3.h>

struct ChatRecord {
    int id;
    std::string userText;
    std::string aiReply;
    std::string emotion;
    std::string timestamp;
};

class MemoryDB {
public:
    explicit MemoryDB(const std::string& dbPath = "data/memory.db");
    ~MemoryDB();

    // 打开数据库并创建表
    bool open();

    // 关闭数据库
    void close();

    // 保存一轮对话
    bool saveChat(const std::string& userText, const std::string& aiReply, const std::string& emotion);

    // 获取最近 n 条对话记录
    std::vector<ChatRecord> getRecentChats(int n = 5);

    // 获取上下文摘要字符串（用于注入 prompt）
    std::string getContext(int n = 5);

    // === 关系系统接口 ===

    // 获取当前好感度（0~100）
    int getAffinity();

    // 增减好感度（钳位 0~100）
    void updateAffinity(int delta);

    // 记录一次情绪
    void recordEmotion(const std::string& emotion);

    // 获取最近 n 轮情绪趋势摘要
    std::string getEmotionTrend(int n = 20);

    // 获取总对话轮数
    int getTotalChats();

    // 增加对话计数
    void incrementChatCount();

private:
    std::string dbPath_;
    sqlite3* db_ = nullptr;

    // 确保 relationship 表有初始行
    void ensureRelationshipRow();
};
