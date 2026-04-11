#pragma once

#include <string>
#include <opencv2/opencv.hpp>

class Camera {
public:
    // sdpPath: SDP 文件路径，默认 /tmp/stream.sdp（H.264 推流）
    explicit Camera(const std::string& sdpPath = "/tmp/stream.sdp");
    ~Camera();

    // 打开视频流
    bool open();

    // 关闭视频流
    void close();

    // 是否已打开
    bool isOpened() const;

    // 获取当前帧，成功返回 true
    bool getFrame(cv::Mat& frame);

private:
    std::string sdpPath_;
    cv::VideoCapture cap_;
    int stderrBackup_ = -1;  // stderr 备份，用于关闭时恢复
};
