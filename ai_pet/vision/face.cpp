#include "vision/face.h"
#include <iostream>

FaceDetector::FaceDetector(const std::string& modelPath)
    : modelPath_(modelPath) {}

bool FaceDetector::load() {
    if (!cascade_.load(modelPath_)) {
        std::cerr << "[FaceDetector] 无法加载模型: " << modelPath_ << std::endl;
        return false;
    }
    loaded_ = true;
    std::cout << "[FaceDetector] 模型已加载: " << modelPath_ << std::endl;
    return true;
}

std::vector<cv::Rect> FaceDetector::detect(const cv::Mat& frame) {
    std::vector<cv::Rect> faces;
    if (!loaded_ || frame.empty()) return faces;

    cv::Mat gray;
    if (frame.channels() == 3) {
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = frame;
    }
    cv::equalizeHist(gray, gray);

    cascade_.detectMultiScale(gray, faces, 1.1, 3, 0, cv::Size(60, 60));
    return faces;
}
