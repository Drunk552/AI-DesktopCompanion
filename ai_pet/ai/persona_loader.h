/**
 * @file ai/persona_loader.h
 * @brief 角色加载模块
 * 
 * 负责从文件系统加载 AI 角色配置。
 * 支持从指定目录加载或自动检测角色。
 */

#pragma once

#include <string>
#include <vector>

/**
 * @brief 角色数据结构
 * @details 存储已加载角色的所有信息
 */
struct PersonaData {
    std::string name;            ///< 角色昵称（从 meta.json 读取）
    std::string personaContent;  ///< persona.md 文件内容
    std::string memoriesContent;  ///< memories.md 文件内容
    bool loaded = false;        ///< 是否成功加载
};

/**
 * @class PersonaLoader
 * @brief 角色加载器
 * @details 负责从文件系统加载角色配置文件
 */
class PersonaLoader {
public:
    /**
     * @brief 从指定目录加载角色
     * @param personaDir 角色目录路径，如 "personas/xiao-mei"
     * @return bool 加载是否成功
     */
    bool load(const std::string& personaDir);
    
    /**
     * @brief 自动检测并加载角色
     * @param personasRoot 角色根目录，默认为 "personas"
     * @return bool 加载是否成功
     * @note 优先加载 active 符号链接指向的角色，否则加载第一个
     */
    bool autoLoad(const std::string& personasRoot = "personas");
    
    /**
     * @brief 获取已加载的角色数据
     * @return const PersonaData& 角色数据引用
     */
    const PersonaData& getData() const;
    
    /**
     * @brief 检查是否已加载角色
     * @return bool 是否已加载
     */
    bool isLoaded() const;
    
    /**
     * @brief 列出所有可用角色
     * @param personasRoot 角色根目录
     * @return std::vector<std::string> 角色名称列表
     */
    static std::vector<std::string> listPersonas(const std::string& personasRoot = "personas");

private:
    /**
     * @brief 读取文件内容
     * @param path 文件路径
     * @return std::string 文件内容
     */
    std::string readFile(const std::string& path);
    
    /**
     * @brief 解析 meta.json 获取角色名称
     * @param metaJson meta.json 内容
     * @return std::string 角色名称
     */
    std::string parseNameFromMeta(const std::string& metaJson);
    
    PersonaData data_;  ///< 角色数据
};
