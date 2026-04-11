#pragma once

#include <string>
#include <opencv2/opencv.hpp>

// 基于 OpenCV 图像特征的轻量表情识别
// 使用人脸区域的几何特征（眉毛、嘴巴区域的亮度/对比度变化）
// 分类为4类情绪：开心、难过、生气、平静
class EmotionRecognizer {
public:
    EmotionRecognizer() = default;
    ~EmotionRecognizer() = default;

    // 初始化（无需模型文件）
    bool load();

    // 识别表情，输入人脸区域图像，返回情绪字符串
    std::string recognize(const cv::Mat& faceROI);

private:
    bool loaded_ = false;

    // 分析人脸上下区域的特征差异来推断情绪
    struct FaceFeatures {
        double mouthOpenness;   // 嘴巴张开程度
        double browTension;     // 眉毛紧张度
        double overallBright;   // 整体亮度
        double contrast;        // 对比度
    };

    FaceFeatures extractFeatures(const cv::Mat& gray);
    std::string classify(const FaceFeatures& feat);
};
