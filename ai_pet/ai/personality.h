/**
 * @file ai/personality.h
 * @brief 人格系统模块
 * 
 * 定义好感度等级和人格演化规则。
 * 根据用户情绪和好感度动态调整 AI 的回复风格。
 */

#pragma once

#include <string>

/**
 * @brief 好感度等级枚举
 * @details 好感度从 0-100，分为 5 个等级
 */
enum class AffinityLevel {
    Stranger,   ///< 陌生（0-20）：冷淡、保持距离
    Distant,    ///< 疏离（21-40）：温柔但克制
    Familiar,   ///< 熟悉（41-60）：偶尔温柔
    Close,      ///< 亲近（61-80）：明显关心
    Intimate    ///< 亲密（81-100）：坦诚依赖
};

/**
 * @brief 人格结构体
 * @details 描述 AI 在当前状态下的回复风格
 */
struct Personality {
    std::string role;   ///< 角色定位
    std::string tone;   ///< 语气风格
    std::string trait;  ///< 性格特征
    std::string style;  ///< 表达风格
};

/**
 * @brief 根据好感度数值获取等级
 * @param affinity 好感度值（0-100）
 * @return AffinityLevel 对应的好感度等级
 */
AffinityLevel getAffinityLevel(int affinity);

/**
 * @brief 获取好感度等级的中文名称
 * @param level 好感度等级
 * @return std::string 等级名称
 */
std::string getAffinityLevelName(AffinityLevel level);

/**
 * @brief 根据好感度和情绪趋势动态生成人格
 * @param affinity 好感度值
 * @param emotionTrend 情绪趋势描述
 * @return Personality 生成的人格配置
 */
Personality evolvePersonality(int affinity, const std::string& emotionTrend);

/**
 * @brief 计算好感度变化值
 * @param userEmotion 用户当前情绪
 * @param userText 用户输入文本
 * @return int 好感度变化值（正值增加，负值减少）
 */
int calculateAffinityDelta(const std::string& userEmotion, const std::string& userText);
