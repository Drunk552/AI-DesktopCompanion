/**
 * @file ai/prompt.h
 * @brief Prompt 构建模块
 * 
 * 负责构造发送给 AI 模型的完整 prompt。
 * 融合动态人格、情绪策略、历史上下文等信息。
 * 
 * @note 支持两种模式：
 *       - Persona 模式：有角色文件时，使用 5 层人格驱动
 *       - 回退模式：无角色文件时，使用硬编码人格系统
 */

#pragma once

#include <string>
#include "ai/persona_loader.h"

/**
 * @brief 构建完整的 AI Prompt
 * 
 * @param userInput 用户输入的文本
 * @param emotion 当前检测到的用户情绪（开心/难过/生气/平静）
 * @param context 历史对话上下文
 * @param affinity 当前好感度值（0-100）
 * @param emotionTrend 情绪趋势描述
 * @param persona 角色数据指针，为空则使用默认人格
 * 
 * @return std::string 构造好的完整 prompt
 */
std::string buildPrompt(const std::string& userInput,
                        const std::string& emotion = "平静",
                        const std::string& context = "",
                        int affinity = 30,
                        const std::string& emotionTrend = "",
                        const PersonaData* persona = nullptr);
