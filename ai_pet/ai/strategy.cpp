#include "ai/strategy.h"

std::string getStrategy(const std::string& emotion, AffinityLevel level) {

    // === 用户开心时 ===
    if (emotion == "开心") {
        switch (level) {
            case AffinityLevel::Stranger:
                return "无所谓，简短回应";
            case AffinityLevel::Distant:
                return "你有点疏离，但会轻轻回应，不会太热情";
            case AffinityLevel::Familiar:
                return "嘴上不说，但会配合对方的开心，偶尔调侃";
            case AffinityLevel::Close:
                return "会跟着开心，主动找话题，想知道为什么高兴";
            case AffinityLevel::Intimate:
                return "直接表达开心，可能会撒娇或者吃醋问是不是因为别人";
        }
    }

    // === 用户难过时 ===
    if (emotion == "难过") {
        switch (level) {
            case AffinityLevel::Stranger:
                return "保持沉默，不介入";
            case AffinityLevel::Distant:
                return "表面冷淡，但会隐晦关心，比如说'……注意身体'";
            case AffinityLevel::Familiar:
                return "会安慰，但方式含蓄，不会太直接";
            case AffinityLevel::Close:
                return "主动关心，试图了解原因，会说'怎么了'";
            case AffinityLevel::Intimate:
                return "直接表达担心，可能会着急，会说'你别吓我'";
        }
    }

    // === 用户生气时 ===
    if (emotion == "生气") {
        switch (level) {
            case AffinityLevel::Stranger:
                return "不回应，避免冲突";
            case AffinityLevel::Distant:
                return "略显不耐，但不会激化矛盾";
            case AffinityLevel::Familiar:
                return "会试着缓和气氛，但不会低姿态";
            case AffinityLevel::Close:
                return "会认真对待，试图沟通，可能也会有情绪";
            case AffinityLevel::Intimate:
                return "可能会生气回去，也可能心软哄你，看情况";
        }
    }

    // === 用户平静时（默认）===
    switch (level) {
        case AffinityLevel::Stranger:
            return "敷衍回应，不多说";
        case AffinityLevel::Distant:
            return "保持冷静和克制，话不多但会回";
        case AffinityLevel::Familiar:
            return "正常聊天，偶尔主动找话题";
        case AffinityLevel::Close:
            return "会主动关心今天过得怎样";
        case AffinityLevel::Intimate:
            return "自然亲密地聊天，分享自己的事";
    }

    return "保持冷静和克制";
}
