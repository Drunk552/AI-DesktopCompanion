#include "vision/vision_pipeline.h"
#include "logger/logger.h"

VisionPipeline::VisionPipeline(FaceDetector& faceDetector, EmotionRecognizer& emotionRecognizer)
    : faceDetector_(faceDetector)
    , emotionRecognizer_(emotionRecognizer) {}

bool VisionPipeline::init() {
    if (!faceDetector_.load()) {
        LOGE("VisionPipeline", "人脸检测模型加载失败");
        return false;
    }
    if (!emotionRecognizer_.load()) {
        LOGE("VisionPipeline", "表情识别模型加载失败");
        return false;
    }
    initialized_ = true;
    return true;
}

VisionResult VisionPipeline::process(const cv::Mat& frame, bool annotate) {
    VisionResult result;
    if (!initialized_ || frame.empty()) {
        return result;
    }

    if (annotate) {
        result.annotatedFrame = frame.clone();
    }

    const auto faces = faceDetector_.detect(frame);
    result.faceDetected = !faces.empty();

    for (const auto& face : faces) {
        if (annotate) {
            cv::rectangle(result.annotatedFrame, face, cv::Scalar(0, 255, 0), 2);
        }

        cv::Mat faceROI = frame(face);
        const std::string emotion = emotionRecognizer_.recognize(faceROI);

        if (annotate) {
            cv::putText(result.annotatedFrame, emotion,
                        cv::Point(face.x, face.y - 10),
                        cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
        }

        if (result.emotion.empty()) {
            result.emotion = emotion;
        }
    }

    return result;
}
