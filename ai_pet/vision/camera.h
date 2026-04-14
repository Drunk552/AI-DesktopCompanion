/**
 * @file vision/camera.h
 * @brief 摄像头视频流管理模块
 * 
 * 负责从 SDP 文件读取 H.264 RTP 视频流，
 * 支持通过 FFmpeg/SDL 在 Windows 端推流，WSL2 端接收。
 */

#pragma once

#include <string>
#include <opencv2/opencv.hpp>

/**
 * @class Camera
 * @brief 摄像头视频流读取器
 * @details 通过 OpenCV 读取 RTP 视频流，支持 H.264 编码格式
 */
class Camera {
public:
    /**
     * @brief 构造函数
     * @param sdpPath SDP 文件路径，默认 "/tmp/stream.sdp"
     */
    explicit Camera(const std::string& sdpPath = "/tmp/stream.sdp");
    
    /**
     * @brief 析构函数，自动关闭视频流
     */
    ~Camera();
    
    /**
     * @brief 打开视频流
     * @return bool 打开是否成功
     * @note 会自动修正 SDP 文件中的端口号
     */
    bool open();
    
    /**
     * @brief 关闭视频流
     */
    void close();
    
    /**
     * @brief 检查视频流是否已打开
     * @return bool 是否已打开
     */
    bool isOpened() const;
    
    /**
     * @brief 获取当前帧
     * @param frame 输出参数，接收帧图像
     * @return bool 是否成功获取帧
     */
    bool getFrame(cv::Mat& frame);

private:
    std::string sdpPath_;      ///< SDP 文件路径
    cv::VideoCapture cap_;     ///< OpenCV 视频捕获对象
    int stderrBackup_ = -1;   ///< stderr 文件描述符备份
};
