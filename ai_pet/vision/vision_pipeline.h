#pragma once

#include "vision/face.h"
#include "vision/emotion.h"
#include <opencv2/opencv.hpp>
#include <string>

struct VisionResult {
    std::string emotion;
    cv::Mat annotatedFrame;
    bool faceDetected = false;
};

class VisionPipeline {
public:
    VisionPipeline(FaceDetector& faceDetector, EmotionRecognizer& emotionRecognizer);

    bool init();
    VisionResult process(const cv::Mat& frame, bool annotate = true);

private:
    FaceDetector& faceDetector_;
    EmotionRecognizer& emotionRecognizer_;
    bool initialized_ = false;
};
