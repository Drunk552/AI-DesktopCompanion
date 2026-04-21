/**
 * @file intelligence/memory/memory_db.h
 * @brief 记忆数据库模块
 *
 * 使用 SQLite 存储对话历史、情绪记录和关系数据。
 * 提供长期记忆功能，让 AI 能够记住之前的对话。
 */

#pragma once

#include <mutex>
#include <sqlite3.h>
#include <string>
#include <vector>

struct ChatRecord {
    int id;
    std::string userText;
    std::string aiReply;
    std::string emotion;
    std::string timestamp;
};

class MemoryDB {
public:
    struct AppStateSnapshot {
        std::string disturbanceMode = "normal";
        std::string lastNotification;
        long long lastNotificationAt = 0;
        long long lastProactiveAt = 0;
        long long lastUserInteractionAt = 0;
        std::string lastProactiveBehavior = "主动关怀";
        int careCount = 0;
        int checkInCount = 0;
        int teaseCount = 0;
        int reminderCount = 0;
        bool allowCare = true;
        bool allowCheckIn = true;
        bool allowTease = true;
        bool allowReminder = true;
        bool silentAtNight = false;
    };

    explicit MemoryDB(const std::string& dbPath = "data/memory.db");
    ~MemoryDB();

    bool open();
    void close();
    bool saveChat(const std::string& userText, const std::string& aiReply, const std::string& emotion);
    std::vector<ChatRecord> getRecentChats(int n = 5);
    std::string getContext(int n = 5);
    int getAffinity();
    void updateAffinity(int delta);
    void recordEmotion(const std::string& emotion);
    std::string getEmotionTrend(int n = 20);
    int getTotalChats();
    void incrementChatCount();
    AppStateSnapshot loadAppState();
    void saveAppState(const AppStateSnapshot& state);

private:
    void ensureRelationshipRow();
    void ensureAppStateRow();

    std::string dbPath_;
    sqlite3* db_ = nullptr;
    mutable std::mutex dbMutex_;
};
