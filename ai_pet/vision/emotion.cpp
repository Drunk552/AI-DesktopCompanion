/**
 * @file vision/emotion.cpp
 * @brief 表情识别模块实现
 * 
 * 优先使用 FERPlus ONNX 模型进行表情识别。
 * 如果模型加载失败，自动回退到几何特征分析方法。
 */

#include "vision/emotion.h"
#include "logger/logger.h"

/// FERPlus 模型的 8 种情绪标签顺序
const std::vector<std::string> EmotionRecognizer::FERPLUS_LABELS = {
    "neutral",    // 0
    "happiness",  // 1
    "surprise",   // 2
    "sadness",    // 3
    "anger",      // 4
    "disgust",    // 5
    "fear",       // 6
    "contempt"    // 7
};

EmotionRecognizer::EmotionRecognizer(const std::string& modelPath)
    : modelPath_(modelPath) {}

bool EmotionRecognizer::load() {
    try {
        net_ = cv::dnn::readNet(modelPath_);
        
        if (!net_.empty()) {
            net_.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
            loaded_ = true;
            netUsed_ = true;
            LOGI("Emotion", "FERPlus ONNX 模型已加载: " + modelPath_);
            return true;
        }
    } catch (const cv::Exception& e) {
        LOGW("Emotion", "ONNX 模型加载失败: " + std::string(e.what()));
    }
    
    LOGW("Emotion", "回退到几何特征分析方法");
    loaded_ = true;
    return true;
}

std::string EmotionRecognizer::mapToEmotion(const std::string& ferplusEmotion) {
    if (ferplusEmotion == "happiness") {
        return "开心";
    }
    if (ferplusEmotion == "sadness" || ferplusEmotion == "fear" || ferplusEmotion == "disgust") {
        return "难过";
    }
    if (ferplusEmotion == "anger" || ferplusEmotion == "contempt") {
        return "生气";
    }
    return "平静";
}

EmotionRecognizer::FaceFeatures EmotionRecognizer::extractFeatures(const cv::Mat& gray) {
    FaceFeatures feat{};
    int h = gray.rows;
    int w = gray.cols;

    cv::Mat browRegion = gray(cv::Rect(0, 0, w, h * 3 / 10));
    cv::Mat mouthRegion = gray(cv::Rect(w / 4, h * 6 / 10, w / 2, h * 4 / 10));

    cv::Scalar mouthMean, mouthStd;
    cv::meanStdDev(mouthRegion, mouthMean, mouthStd);
    feat.mouthOpenness = mouthStd[0];

    cv::Mat browGrad;
    cv::Sobel(browRegion, browGrad, CV_64F, 0, 1);
    feat.browTension = cv::mean(cv::abs(browGrad))[0];

    feat.overallBright = cv::mean(gray)[0];

    cv::Mat upperHalf = gray(cv::Rect(0, 0, w, h / 2));
    cv::Mat lowerHalf = gray(cv::Rect(0, h / 2, w, h / 2));
    feat.contrast = std::abs(cv::mean(upperHalf)[0] - cv::mean(lowerHalf)[0]);

    return feat;
}

std::string EmotionRecognizer::classify(const FaceFeatures& feat) {
    if (feat.mouthOpenness > 35.0 && feat.overallBright > 110.0) {
        return "开心";
    }
    if (feat.browTension > 30.0 && feat.contrast > 25.0) {
        return "生气";
    }
    if (feat.overallBright < 100.0 && feat.mouthOpenness < 25.0) {
        return "难过";
    }
    return "平静";
}

std::string EmotionRecognizer::recognize(const cv::Mat& faceROI) {
    if (faceROI.empty()) {
        return "平静";
    }

    cv::Mat gray;
    if (faceROI.channels() == 3) {
        cv::cvtColor(faceROI, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = faceROI.clone();
    }

    cv::resize(gray, gray, cv::Size(64, 64));
    cv::equalizeHist(gray, gray);

    if (netUsed_) {
        try {
            cv::Mat input = gray.clone();
            input.convertTo(input, CV_32FC1, 1.0 / 255.0);
            cv::Mat blob = cv::dnn::blobFromImage(input, 1.0, cv::Size(),
                                                  cv::Scalar(0), true, false);
            net_.setInput(blob);
            cv::Mat output = net_.forward();

            cv::Point maxLoc;
            double maxVal;
            cv::minMaxLoc(output, nullptr, &maxVal, nullptr, &maxLoc);

            int emotionIdx = maxLoc.x;
            if (emotionIdx >= 0 && emotionIdx < static_cast<int>(FERPLUS_LABELS.size())) {
                return mapToEmotion(FERPLUS_LABELS[emotionIdx]);
            }
        } catch (const cv::Exception& e) {
            LOGW("Emotion", "ONNX 推理失败，使用几何特征: " + std::string(e.what()));
            netUsed_ = false;
        }
    }

    if (!netUsed_) {
        FaceFeatures feat = extractFeatures(gray);
        return classify(feat);
    }

    return "平静";
}
