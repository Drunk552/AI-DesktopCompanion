/**
 * @file vision/emotion.h
 * @brief 表情识别模块
 * 
 * 优先使用 FERPlus ONNX 模型进行表情识别。
 * 如果模型加载失败，自动回退到几何特征分析方法。
 * 支持 8 种情绪分类，映射为 4 种情绪：开心、难过、生气、平静。
 */

#pragma once

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

/**
 * @class EmotionRecognizer
 * @brief 表情识别器
 * @details 优先使用 OpenCV DNN 加载 FERPlus ONNX 模型，失败时回退到几何特征分析
 */
class EmotionRecognizer {
public:
    /**
     * @brief 构造函数
     * @param modelPath ONNX 模型路径，为空使用默认路径
     */
    explicit EmotionRecognizer(const std::string& modelPath = "models/emotion-ferplus-8.onnx");
    
    /**
     * @brief 析构函数
     */
    ~EmotionRecognizer() = default;
    
    /**
     * @brief 加载模型
     * @return bool 加载是否成功（即使 ONNX 失败也返回 true，回退到几何特征）
     */
    bool load();
    
    /**
     * @brief 识别表情
     * @param faceROI 输入参数，人脸区域图像
     * @return std::string 识别到的情绪（开心/难过/生气/平静）
     */
    std::string recognize(const cv::Mat& faceROI);
    
    /**
     * @brief 检查模型是否已加载
     * @return bool 是否已初始化
     */
    bool isLoaded() const { return loaded_; }

private:
    /**
     * @brief 表情特征结构体（几何特征方法）
     */
    struct FaceFeatures {
        double mouthOpenness;   ///< 嘴巴张开程度
        double browTension;     ///< 眉毛紧张度
        double overallBright;   ///< 整体亮度
        double contrast;        ///< 对比度
    };
    
    /**
     * @brief 提取人脸几何特征
     * @param gray 灰度化后的人脸图像
     * @return FaceFeatures 提取的特征
     */
    FaceFeatures extractFeatures(const cv::Mat& gray);
    
    /**
     * @brief 根据几何特征分类情绪
     * @param feat 特征数据
     * @return std::string 情绪类别
     */
    std::string classify(const FaceFeatures& feat);
    
    /**
     * @brief 将 FERPlus 的 8 类情绪映射到项目的 4 类情绪
     * @param ferplusEmotion FERPlus 情绪标签
     * @return std::string 映射后的情绪
     */
    std::string mapToEmotion(const std::string& ferplusEmotion);
    
    std::string modelPath_;  ///< ONNX 模型路径
    cv::dnn::Net net_;      ///< DNN 网络
    bool loaded_ = false;   ///< 是否已初始化
    bool netUsed_ = false;  ///< 是否使用 ONNX 模型
    
    /// FERPlus 情绪标签顺序（对应模型输出）
    static const std::vector<std::string> FERPLUS_LABELS;
};
