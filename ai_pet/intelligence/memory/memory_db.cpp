#include "intelligence/memory/memory_db.h"
#include "shared/logger/logger.h"
#include <algorithm>
#include <sstream>
#include <sys/stat.h>

static std::string safeColumnText(sqlite3_stmt* stmt, int col) {
    const char* p = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col));
    return p ? p : "";
}

static std::string compact_for_context(const std::string& text, size_t maxLen = 160) {
    std::string compact;
    compact.reserve(std::min(text.size(), maxLen));

    bool lastWasSpace = false;
    for (char ch : text) {
        const bool isSpace = ch == '\n' || ch == '\r' || ch == '\t' || ch == ' ';
        if (isSpace) {
            if (!lastWasSpace) {
                compact.push_back(' ');
            }
            lastWasSpace = true;
        } else {
            compact.push_back(ch);
            lastWasSpace = false;
        }

        if (compact.size() >= maxLen) {
            break;
        }
    }

    while (!compact.empty() && compact.back() == ' ') {
        compact.pop_back();
    }

    if (text.size() > compact.size()) {
        compact += "...";
    }
    return compact;
}

MemoryDB::MemoryDB(const std::string& dbPath)
    : dbPath_(dbPath) {}

MemoryDB::~MemoryDB() {
    close();
}

bool MemoryDB::open() {
    std::lock_guard<std::mutex> lock(dbMutex_);

    std::string dir = dbPath_.substr(0, dbPath_.find_last_of('/'));
    if (!dir.empty() && dir != dbPath_) {
        mkdir(dir.c_str(), 0755);
    }

    int rc = sqlite3_open(dbPath_.c_str(), &db_);
    if (rc != SQLITE_OK) {
        LOGE("Memory", "无法打开数据库: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }

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
        LOGE("Memory", "创建 chat 表失败: " + std::string(errMsg));
        sqlite3_free(errMsg);
        return false;
    }

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
        LOGE("Memory", "创建 relationship 表失败: " + std::string(errMsg));
        sqlite3_free(errMsg);
        return false;
    }

    const char* sqlEmo =
        "CREATE TABLE IF NOT EXISTS emotion_history ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  emotion TEXT NOT NULL,"
        "  timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";
    rc = sqlite3_exec(db_, sqlEmo, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        LOGE("Memory", "创建 emotion_history 表失败: " + std::string(errMsg));
        sqlite3_free(errMsg);
        return false;
    }

    const char* sqlIdxChat = "CREATE INDEX IF NOT EXISTS idx_chat_timestamp ON chat(timestamp);";
    rc = sqlite3_exec(db_, sqlIdxChat, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK && errMsg) {
        sqlite3_free(errMsg);
        errMsg = nullptr;
    }

    const char* sqlIdxEmo = "CREATE INDEX IF NOT EXISTS idx_emotion_timestamp ON emotion_history(timestamp);";
    rc = sqlite3_exec(db_, sqlIdxEmo, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK && errMsg) {
        sqlite3_free(errMsg);
        errMsg = nullptr;
    }

    ensureRelationshipRow();
    const char* sqlAppState =
        "CREATE TABLE IF NOT EXISTS app_state ("
        "  id INTEGER PRIMARY KEY CHECK (id = 1),"
        "  disturbance_mode TEXT DEFAULT 'normal',"
        "  last_notification TEXT DEFAULT '',"
        "  last_notification_at INTEGER DEFAULT 0,"
        "  last_proactive_at INTEGER DEFAULT 0,"
        "  last_user_interaction_at INTEGER DEFAULT 0,"
        "  last_proactive_behavior TEXT DEFAULT '主动关怀',"
        "  care_count INTEGER DEFAULT 0,"
        "  checkin_count INTEGER DEFAULT 0,"
        "  tease_count INTEGER DEFAULT 0,"
        "  reminder_count INTEGER DEFAULT 0,"
        "  allow_care INTEGER DEFAULT 1,"
        "  allow_checkin INTEGER DEFAULT 1,"
        "  allow_tease INTEGER DEFAULT 1,"
        "  allow_reminder INTEGER DEFAULT 1,"
        "  silent_at_night INTEGER DEFAULT 0"
        ");";
    rc = sqlite3_exec(db_, sqlAppState, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        LOGE("Memory", "创建 app_state 表失败: " + std::string(errMsg));
        sqlite3_free(errMsg);
        return false;
    }
    ensureAppStateRow();

    LOGI("Memory", "数据库已打开: " + dbPath_);
    return true;
}

void MemoryDB::close() {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool MemoryDB::saveChat(const std::string& userText, const std::string& aiReply, const std::string& emotion) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char* sql = "INSERT INTO chat (user_text, ai_reply, emotion) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOGE("Memory", "准备语句失败: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }

    sqlite3_bind_text(stmt, 1, userText.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, aiReply.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, emotion.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        LOGE("Memory", "保存对话失败: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    return true;
}

std::vector<ChatRecord> MemoryDB::getRecentChats(int n) {
    std::lock_guard<std::mutex> lock(dbMutex_);
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
        rec.userText = safeColumnText(stmt, 1);
        rec.aiReply = safeColumnText(stmt, 2);
        rec.emotion = safeColumnText(stmt, 3);
        rec.timestamp = safeColumnText(stmt, 4);
        records.push_back(rec);
    }
    sqlite3_finalize(stmt);

    std::reverse(records.begin(), records.end());
    return records;
}

std::string MemoryDB::getContext(int n) {
    auto records = getRecentChats(n);
    if (records.empty()) return "";

    std::string context = "以下是之前的对话记录：\n";
    for (const auto& rec : records) {
        context += "用户（" + rec.emotion + "）：" + rec.userText + "\n";
        context += "你：" + compact_for_context(rec.aiReply) + "\n";
    }
    return context;
}

void MemoryDB::ensureRelationshipRow() {
    if (!db_) return;
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
        LOGI("Memory", "初始化关系数据（好感度: 30）");
    }
}

void MemoryDB::ensureAppStateRow() {
    if (!db_) return;
    const char* sql = "SELECT COUNT(*) FROM app_state WHERE id = 1;";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return;
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (count == 0) {
        const char* insert = "INSERT INTO app_state (id) VALUES (1);";
        sqlite3_exec(db_, insert, nullptr, nullptr, nullptr);
    }
}

int MemoryDB::getAffinity() {
    std::lock_guard<std::mutex> lock(dbMutex_);
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
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return;

    int current = 30;
    {
        const char* sql = "SELECT affinity FROM relationship ORDER BY id LIMIT 1;";
        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        if (rc == SQLITE_OK && sqlite3_step(stmt) == SQLITE_ROW) {
            current = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    int newVal = current + delta;
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
        LOGI("Memory", "好感度: " + std::to_string(current) + " -> " + std::to_string(newVal) + " (" + (delta > 0 ? "+" : "") + std::to_string(delta) + ")");
    }
}

void MemoryDB::recordEmotion(const std::string& emotion) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return;
    const char* sql = "INSERT INTO emotion_history (emotion) VALUES (?);";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, emotion.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    const char* sql2 = "UPDATE relationship SET last_emotion = ?, updated_at = CURRENT_TIMESTAMP WHERE id = 1;";
    rc = sqlite3_prepare_v2(db_, sql2, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, emotion.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::string MemoryDB::getEmotionTrend(int n) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return "无情绪数据";

    const char* sql = "SELECT emotion FROM emotion_history ORDER BY id DESC LIMIT ?;";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return "无情绪数据";
    sqlite3_bind_int(stmt, 1, n);

    int happy = 0, sad = 0, angry = 0, calm = 0, total = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string emo = safeColumnText(stmt, 0);
        if (emo == "开心") happy++;
        else if (emo == "难过") sad++;
        else if (emo == "生气") angry++;
        else calm++;
        total++;
    }
    sqlite3_finalize(stmt);

    if (total == 0) return "无情绪数据";

    std::string trend = "偏平静";
    int maxCount = calm;
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
    std::lock_guard<std::mutex> lock(dbMutex_);
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
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return;
    const char* sql = "UPDATE relationship SET total_chats = total_chats + 1, updated_at = CURRENT_TIMESTAMP WHERE id = 1;";
    sqlite3_exec(db_, sql, nullptr, nullptr, nullptr);
}

MemoryDB::AppStateSnapshot MemoryDB::loadAppState() {
    std::lock_guard<std::mutex> lock(dbMutex_);
    AppStateSnapshot state;
    if (!db_) return state;
    const char* sql =
        "SELECT disturbance_mode,last_notification,last_notification_at,last_proactive_at,last_user_interaction_at,"
        "last_proactive_behavior,care_count,checkin_count,tease_count,reminder_count,allow_care,allow_checkin,allow_tease,allow_reminder,silent_at_night "
        "FROM app_state WHERE id = 1;";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return state;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        state.disturbanceMode = safeColumnText(stmt, 0);
        state.lastNotification = safeColumnText(stmt, 1);
        state.lastNotificationAt = sqlite3_column_int64(stmt, 2);
        state.lastProactiveAt = sqlite3_column_int64(stmt, 3);
        state.lastUserInteractionAt = sqlite3_column_int64(stmt, 4);
        state.lastProactiveBehavior = safeColumnText(stmt, 5);
        state.careCount = sqlite3_column_int(stmt, 6);
        state.checkInCount = sqlite3_column_int(stmt, 7);
        state.teaseCount = sqlite3_column_int(stmt, 8);
        state.reminderCount = sqlite3_column_int(stmt, 9);
        state.allowCare = sqlite3_column_int(stmt, 10) != 0;
        state.allowCheckIn = sqlite3_column_int(stmt, 11) != 0;
        state.allowTease = sqlite3_column_int(stmt, 12) != 0;
        state.allowReminder = sqlite3_column_int(stmt, 13) != 0;
        state.silentAtNight = sqlite3_column_int(stmt, 14) != 0;
    }
    sqlite3_finalize(stmt);
    return state;
}

void MemoryDB::saveAppState(const AppStateSnapshot& state) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return;
    const char* sql =
        "UPDATE app_state SET disturbance_mode=?,last_notification=?,last_notification_at=?,last_proactive_at=?,last_user_interaction_at=?,"
        "last_proactive_behavior=?,care_count=?,checkin_count=?,tease_count=?,reminder_count=?,allow_care=?,allow_checkin=?,allow_tease=?,allow_reminder=?,silent_at_night=? WHERE id = 1;";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, state.disturbanceMode.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, state.lastNotification.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 3, state.lastNotificationAt);
    sqlite3_bind_int64(stmt, 4, state.lastProactiveAt);
    sqlite3_bind_int64(stmt, 5, state.lastUserInteractionAt);
    sqlite3_bind_text(stmt, 6, state.lastProactiveBehavior.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 7, state.careCount);
    sqlite3_bind_int(stmt, 8, state.checkInCount);
    sqlite3_bind_int(stmt, 9, state.teaseCount);
    sqlite3_bind_int(stmt, 10, state.reminderCount);
    sqlite3_bind_int(stmt, 11, state.allowCare ? 1 : 0);
    sqlite3_bind_int(stmt, 12, state.allowCheckIn ? 1 : 0);
    sqlite3_bind_int(stmt, 13, state.allowTease ? 1 : 0);
    sqlite3_bind_int(stmt, 14, state.allowReminder ? 1 : 0);
    sqlite3_bind_int(stmt, 15, state.silentAtNight ? 1 : 0);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
