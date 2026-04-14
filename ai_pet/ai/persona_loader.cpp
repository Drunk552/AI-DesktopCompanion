#include "ai/persona_loader.h"
#include "logger/logger.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

std::string PersonaLoader::readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

std::string PersonaLoader::parseNameFromMeta(const std::string& metaJson) {
    try {
        auto j = nlohmann::json::parse(metaJson);
        if (j.contains("name")) {
            return j["name"].get<std::string>();
        }
    } catch (const nlohmann::json::exception& e) {
        LOGE("Persona", "meta.json 解析失败: " + std::string(e.what()));
    }
    return "";
}

bool PersonaLoader::load(const std::string& personaDir) {
    std::string personaPath = personaDir + "/persona.md";
    if (!fs::exists(personaPath)) {
        LOGE("Persona", "未找到 persona.md: " + personaPath);
        return false;
    }

    data_.personaContent = readFile(personaPath);
    if (data_.personaContent.empty()) {
        LOGE("Persona", "persona.md 为空: " + personaPath);
        return false;
    }

    std::string memoriesPath = personaDir + "/memories.md";
    if (fs::exists(memoriesPath)) {
        data_.memoriesContent = readFile(memoriesPath);
    }

    std::string metaPath = personaDir + "/meta.json";
    if (fs::exists(metaPath)) {
        std::string metaContent = readFile(metaPath);
        if (!metaContent.empty()) {
            data_.name = parseNameFromMeta(metaContent);
        }
    }

    if (data_.name.empty()) {
        data_.name = fs::path(personaDir).filename().string();
    }

    data_.loaded = true;
    LOGI("Persona", "角色已加载: " + data_.name +
         " (persona: " + std::to_string(data_.personaContent.size()) + " bytes" +
         ", memories: " + std::to_string(data_.memoriesContent.size()) + " bytes)");
    return true;
}

bool PersonaLoader::autoLoad(const std::string& personasRoot) {
    if (!fs::exists(personasRoot) || !fs::is_directory(personasRoot)) {
        return false;
    }

    // 优先检查 active 符号链接
    std::string activePath = personasRoot + "/active";
    if (fs::exists(activePath)) {
        std::string resolved = fs::canonical(activePath).string();
        if (fs::is_directory(resolved) && fs::exists(resolved + "/persona.md")) {
            return load(resolved);
        }
    }

    // 否则扫描第一个含 persona.md 的子目录
    for (const auto& entry : fs::directory_iterator(personasRoot)) {
        if (entry.is_directory()) {
            std::string dir = entry.path().string();
            if (fs::exists(dir + "/persona.md")) {
                return load(dir);
            }
        }
    }

    return false;
}

const PersonaData& PersonaLoader::getData() const {
    return data_;
}

bool PersonaLoader::isLoaded() const {
    return data_.loaded;
}

std::vector<std::string> PersonaLoader::listPersonas(const std::string& personasRoot) {
    std::vector<std::string> names;

    if (!fs::exists(personasRoot) || !fs::is_directory(personasRoot)) {
        return names;
    }

    for (const auto& entry : fs::directory_iterator(personasRoot)) {
        if (entry.is_directory()) {
            std::string dir = entry.path().string();
            if (fs::exists(dir + "/persona.md")) {
                // 尝试从 meta.json 获取名字
                std::string metaPath = dir + "/meta.json";
                std::string displayName = entry.path().filename().string();

                if (fs::exists(metaPath)) {
                    std::ifstream f(metaPath);
                    if (f.is_open()) {
                        std::ostringstream ss;
                        ss << f.rdbuf();
                        try {
                            auto j = nlohmann::json::parse(ss.str());
                            if (j.contains("name")) {
                                displayName = j["name"].get<std::string>();
                            }
                        } catch (...) {}
                    }
                }

                std::string slug = entry.path().filename().string();
                names.push_back(slug + " (" + displayName + ")");
            }
        }
    }

    return names;
}
