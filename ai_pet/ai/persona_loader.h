#pragma once

#include <string>
#include <vector>

struct PersonaData {
    std::string name;           // 角色昵称（从 meta.json）
    std::string personaContent; // persona.md 全文
    std::string memoriesContent;// memories.md 全文
    bool loaded = false;        // 是否成功加载
};

class PersonaLoader {
public:
    // 从指定目录加载角色（如 "personas/xiao-mei"）
    bool load(const std::string& personaDir);

    // 从 personas/ 目录自动检测并加载（优先 active 链接，否则加载第一个）
    bool autoLoad(const std::string& personasRoot = "personas");

    // 获取加载的角色数据
    const PersonaData& getData() const;

    // 是否已加载角色
    bool isLoaded() const;

    // 列出可用角色
    static std::vector<std::string> listPersonas(const std::string& personasRoot = "personas");

private:
    PersonaData data_;
    std::string readFile(const std::string& path);
    std::string parseNameFromMeta(const std::string& metaJson);
};
