#include "memory/sqlite.h"
#include <iostream>
#include <algorithm>
#include <sys/stat.h>

MemoryDB::MemoryDB(const std::string& dbPath)
    : dbPath_(dbPath) {}

MemoryDB::~MemoryDB() {
    close();
}

bool MemoryDB::open() {
    // 确保 data 目录存在
    std::string dir = dbPath_.substr(0, dbPath_.find_last_of('/'));
    if (!dir.empty()) {
        mkdir(dir.c_str(), 0755);
    }

    int rc = sqlite3_open(dbPath_.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "[MemoryDB] 无法打开数据库: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    // 创建 chat 表
    const char* sqlChat =
        "CREATE TABLE IF NOT EXISTS chat ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  user_text TEXT NOT NULL,"
        "  ai_reply TEXT NOT NULL,"
        "  emotion TEXT DEFAULT '平静',"
        "  timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";

    char* errMsg = nullptr;
    rc = sqlite3_exec(db_, sqlChat, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "[MemoryDB] 创建 chat 表失败: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    // 创建 relationship 表
    const char* sqlRel =
        "CREATE TABLE IF NOT EXISTS relationship ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  affinity INTEGER DEFAULT 30,"
        "  total_chats INTEGER DEFAULT 0,"
        "  last_emotion TEXT DEFAULT '平静',"
        "  updated_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";
    rc = sqlite3_exec(db_, sqlRel, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "[MemoryDB] 创建 relationship 表失败: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    // 创建 emotion_history 表
    const char* sqlEmo =
        "CREATE TABLE IF NOT EXISTS emotion_history ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  emotion TEXT NOT NULL,"
        "  timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";
    rc = sqlite3_exec(db_, sqlEmo, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "[MemoryDB] 创建 emotion_history 表失败: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    // 确保 relationship 表有初始行
    ensureRelationshipRow();

    std::cout << "[MemoryDB] 数据库已打开: " << dbPath_ << std::endl;
    return true;
}

void MemoryDB::close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool MemoryDB::saveChat(const std::string& userText, const std::string& aiReply, const std::string& emotion) {
    if (!db_) return false;

    const char* sql = "INSERT INTO chat (user_text, ai_reply, emotion) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "[MemoryDB] 准备语句失败: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, userText.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, aiReply.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, emotion.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "[MemoryDB] 保存对话失败: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    return true;
}

std::vector<ChatRecord> MemoryDB::getRecentChats(int n) {
    std::vector<ChatRecord> records;
    if (!db_) return records;

    const char* sql = "SELECT id, user_text, ai_reply, emotion, timestamp FROM chat ORDER BY id DESC LIMIT ?;";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return records;

    sqlite3_bind_int(stmt, 1, n);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ChatRecord rec;
        rec.id = sqlite3_column_int(stmt, 0);
        rec.userText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        rec.aiReply = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        rec.emotion = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        rec.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        records.push_back(rec);
    }
    sqlite3_finalize(stmt);

    // 反转为时间正序
    std::reverse(records.begin(), records.end());
    return records;
}

std::string MemoryDB::getContext(int n) {
    auto records = getRecentChats(n);
    if (records.empty()) return "";

    std::string context = "以下是之前的对话记录：\n";
    for (const auto& rec : records) {
        context += "用户（" + rec.emotion + "）：" + rec.userText + "\n";
        context += "你：" + rec.aiReply + "\n";
    }
    return context;
}

// ============================================================
// 关系系统
// ============================================================

void MemoryDB::ensureRelationshipRow() {
    if (!db_) return;
    // 检查是否已有数据
    const char* sql = "SELECT COUNT(*) FROM relationship;";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return;
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (count == 0) {
        const char* insert = "INSERT INTO relationship (affinity, total_chats) VALUES (30, 0);";
        sqlite3_exec(db_, insert, nullptr, nullptr, nullptr);
        std::cout << "[MemoryDB] 初始化关系数据（好感度: 30）" << std::endl;
    }
}

int MemoryDB::getAffinity() {
    if (!db_) return 30;
    const char* sql = "SELECT affinity FROM relationship ORDER BY id LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return 30;
    int affinity = 30;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        affinity = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return affinity;
}

void MemoryDB::updateAffinity(int delta) {
    if (!db_) return;
    int current = getAffinity();
    int newVal = current + delta;
    // 钳位 0~100
    if (newVal < 0) newVal = 0;
    if (newVal > 100) newVal = 100;

    const char* sql = "UPDATE relationship SET affinity = ?, updated_at = CURRENT_TIMESTAMP WHERE id = 1;";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return;
    sqlite3_bind_int(stmt, 1, newVal);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (delta != 0) {
        std::cout << "[关系] 好感度: " << current << " -> " << newVal
                  << " (" << (delta > 0 ? "+" : "") << delta << ")" << std::endl;
    }
}

void MemoryDB::recordEmotion(const std::string& emotion) {
    if (!db_) return;
    const char* sql = "INSERT INTO emotion_history (emotion) VALUES (?);";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, emotion.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    // 同时更新 relationship 的 last_emotion
    const char* sql2 = "UPDATE relationship SET last_emotion = ?, updated_at = CURRENT_TIMESTAMP WHERE id = 1;";
    rc = sqlite3_prepare_v2(db_, sql2, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, emotion.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::string MemoryDB::getEmotionTrend(int n) {
    if (!db_) return "无情绪数据";

    const char* sql = "SELECT emotion FROM emotion_history ORDER BY id DESC LIMIT ?;";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return "无情绪数据";
    sqlite3_bind_int(stmt, 1, n);

    int happy = 0, sad = 0, angry = 0, calm = 0, total = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string emo = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (emo == "开心") happy++;
        else if (emo == "难过") sad++;
        else if (emo == "生气") angry++;
        else calm++;
        total++;
    }
    sqlite3_finalize(stmt);

    if (total == 0) return "无情绪数据";

    // 找出主导情绪
    std::string trend;
    int maxCount = calm;
    trend = "偏平静";
    if (happy > maxCount) { maxCount = happy; trend = "偏开心"; }
    if (sad > maxCount) { maxCount = sad; trend = "偏难过"; }
    if (angry > maxCount) { maxCount = angry; trend = "偏生气"; }

    return "最近" + std::to_string(total) + "轮对话情绪" + trend
         + "（开心:" + std::to_string(happy)
         + " 难过:" + std::to_string(sad)
         + " 生气:" + std::to_string(angry)
         + " 平静:" + std::to_string(calm) + "）";
}

int MemoryDB::getTotalChats() {
    if (!db_) return 0;
    const char* sql = "SELECT total_chats FROM relationship ORDER BY id LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return 0;
    int total = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        total = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return total;
}

void MemoryDB::incrementChatCount() {
    if (!db_) return;
    const char* sql = "UPDATE relationship SET total_chats = total_chats + 1, updated_at = CURRENT_TIMESTAMP WHERE id = 1;";
    sqlite3_exec(db_, sql, nullptr, nullptr, nullptr);
}
