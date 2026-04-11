#pragma once

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

class FaceDetector {
public:
    explicit FaceDetector(const std::string& modelPath = "models/haarcascade_frontalface_default.xml");
    ~FaceDetector() = default;

    // 加载模型
    bool load();

    // 检测人脸，返回人脸区域列表
    std::vector<cv::Rect> detect(const cv::Mat& frame);

private:
    std::string modelPath_;
    cv::CascadeClassifier cascade_;
    bool loaded_ = false;
};
