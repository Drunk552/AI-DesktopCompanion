/**
 * @file ai/strategy.h
 * @brief 情绪回应策略模块
 * 
 * 根据用户情绪和好感度等级，
 * 确定 AI 应该采取的回应策略。
 */

#pragma once

#include "ai/personality.h"
#include <string>

/**
 * @brief 获取情绪回应策略
 * 
 * @param emotion 用户当前情绪（开心/难过/生气/平静）
 * @param level 当前好感度等级
 * 
 * @return std::string 策略描述字符串
 * 
 * @note 策略矩阵：4 种情绪 × 5 种好感度等级 = 20 种不同策略
 */
std::string getStrategy(const std::string& emotion, AffinityLevel level);
