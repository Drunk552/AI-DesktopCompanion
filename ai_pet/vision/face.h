/**
 * @file vision/face.h
 * @brief 人脸检测模块
 * 
 * 使用 Haar 级联分类器进行人脸检测，
 * 实时从视频帧中检测人脸区域。
 */

#pragma once

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

/**
 * @class FaceDetector
 * @brief 人脸检测器
 * @details 使用 OpenCV 的 CascadeClassifier 进行人脸检测
 */
class FaceDetector {
public:
    /**
     * @brief 构造函数
     * @param modelPath Haar 级联分类器模型文件路径
     */
    explicit FaceDetector(const std::string& modelPath = "models/haarcascade_frontalface_default.xml");
    
    /**
     * @brief 析构函数
     */
    ~FaceDetector() = default;
    
    /**
     * @brief 加载人脸检测模型
     * @return bool 加载是否成功
     */
    bool load();
    
    /**
     * @brief 检测人脸
     * @param frame 输入图像帧
     * @return std::vector<cv::Rect> 检测到的人脸区域列表
     */
    std::vector<cv::Rect> detect(const cv::Mat& frame);

private:
    std::string modelPath_;       ///< 模型文件路径
    cv::CascadeClassifier cascade_;///< 级联分类器
    bool loaded_ = false;        ///< 模型是否已加载
};
