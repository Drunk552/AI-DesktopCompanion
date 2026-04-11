#include "vision/emotion.h"
#include <iostream>

bool EmotionRecognizer::load() {
    loaded_ = true;
    std::cout << "[EmotionRecognizer] 特征分析器已初始化（无需模型文件）" << std::endl;
    return true;
}

EmotionRecognizer::FaceFeatures EmotionRecognizer::extractFeatures(const cv::Mat& gray) {
    FaceFeatures feat{};
    int h = gray.rows;
    int w = gray.cols;

    // 将人脸分为上、中、下三个区域
    // 上部：眉毛区域 (0~30%)
    // 中部：眼睛区域 (25%~55%)
    // 下部：嘴巴区域 (60%~100%)
    cv::Mat browRegion = gray(cv::Rect(0, 0, w, h * 3 / 10));
    cv::Mat eyeRegion  = gray(cv::Rect(0, h / 4, w, h * 3 / 10));
    cv::Mat mouthRegion = gray(cv::Rect(w / 4, h * 6 / 10, w / 2, h * 4 / 10));

    // 嘴巴张开程度：嘴巴区域的标准差（张嘴时变化大）
    cv::Scalar mouthMean, mouthStd;
    cv::meanStdDev(mouthRegion, mouthMean, mouthStd);
    feat.mouthOpenness = mouthStd[0];

    // 眉毛紧张度：眉毛区域的梯度强度
    cv::Mat browGrad;
    cv::Sobel(browRegion, browGrad, CV_64F, 0, 1);
    feat.browTension = cv::mean(cv::abs(browGrad))[0];

    // 整体亮度
    feat.overallBright = cv::mean(gray)[0];

    // 对比度：上半脸和下半脸的亮度差
    cv::Mat upperHalf = gray(cv::Rect(0, 0, w, h / 2));
    cv::Mat lowerHalf = gray(cv::Rect(0, h / 2, w, h / 2));
    feat.contrast = std::abs(cv::mean(upperHalf)[0] - cv::mean(lowerHalf)[0]);

    return feat;
}

std::string EmotionRecognizer::classify(const FaceFeatures& feat) {
    // 基于特征阈值的简单分类规则
    // 嘴巴张开 + 亮度较高 -> 开心
    if (feat.mouthOpenness > 35.0 && feat.overallBright > 110.0) {
        return "开心";
    }
    // 眉毛紧缩 + 对比度高 -> 生气
    if (feat.browTension > 30.0 && feat.contrast > 25.0) {
        return "生气";
    }
    // 亮度低 + 嘴巴区域变化小 -> 难过
    if (feat.overallBright < 100.0 && feat.mouthOpenness < 25.0) {
        return "难过";
    }
    return "平静";
}

std::string EmotionRecognizer::recognize(const cv::Mat& faceROI) {
    if (!loaded_ || faceROI.empty()) return "平静";

    // 灰度化
    cv::Mat gray;
    if (faceROI.channels() == 3) {
        cv::cvtColor(faceROI, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = faceROI.clone();
    }

    // 统一尺寸
    cv::resize(gray, gray, cv::Size(64, 64));
    cv::equalizeHist(gray, gray);

    // 提取特征并分类
    FaceFeatures feat = extractFeatures(gray);
    return classify(feat);
}
