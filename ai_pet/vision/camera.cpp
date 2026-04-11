#include "vision/camera.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>

extern "C" {
#include <libavutil/log.h>
}

// 静默 FFmpeg 日志回调（什么都不输出）
static void silentLogCallback(void*, int, const char*, va_list) {}

// 修正 SDP 文件中的端口号（FFmpeg 生成的 SDP 可能端口为 0）
static void fixSdpPort(const std::string& sdpPath, int port) {
    std::ifstream in(sdpPath);
    if (!in.is_open()) return;

    std::stringstream buf;
    buf << in.rdbuf();
    in.close();

    std::string content = buf.str();
    // 查找 "m=video 0 " 并替换为正确端口
    std::string search = "m=video 0 ";
    std::string replace = "m=video " + std::to_string(port) + " ";
    auto pos = content.find(search);
    if (pos != std::string::npos) {
        content.replace(pos, search.length(), replace);
        std::ofstream out(sdpPath);
        out << content;
        out.close();
        std::cout << "[Camera] 已修正 SDP 端口: 0 -> " << port << std::endl;
    }
}

Camera::Camera(const std::string& sdpPath)
    : sdpPath_(sdpPath) {}

Camera::~Camera() {
    close();
}

bool Camera::open() {
    // 完全禁止 FFmpeg 日志输出
    av_log_set_level(AV_LOG_QUIET);
    av_log_set_callback(silentLogCallback);

    // 修正 SDP 端口号（Windows FFmpeg 可能生成 port=0）
    fixSdpPort(sdpPath_, 5004);

    // 重定向 stderr 到 /dev/null（持续拑截 OpenCV 内部 FFmpeg 解码错误输出）
    stderrBackup_ = dup(STDERR_FILENO);
    int devNull = ::open("/dev/null", O_WRONLY);
    dup2(devNull, STDERR_FILENO);
    ::close(devNull);

    // 通过环境变量设置 FFmpeg 低延迟参数
    setenv("OPENCV_FFMPEG_CAPTURE_OPTIONS",
           "protocol_whitelist;file,rtp,udp,crypto,data|"
           "fflags;nobuffer|"
           "flags;low_delay|"
           "analyzeduration;0|"
           "probesize;32768|"
           "strict;-2",
           1);

    cap_.open(sdpPath_, cv::CAP_FFMPEG);

    if (!cap_.isOpened()) {
        // 恢复 stderr 再输出错误
        dup2(stderrBackup_, STDERR_FILENO);
        ::close(stderrBackup_);
        stderrBackup_ = -1;
        std::cerr << "[Camera] 无法打开视频流: " << sdpPath_ << std::endl;
        std::cerr << "[Camera] 请检查: 1) SDP文件是否存在  2) Windows端是否在推流  3) 网络是否连通" << std::endl;
        return false;
    }
    // 设置缓冲区为1帧，降低延迟
    cap_.set(cv::CAP_PROP_BUFFERSIZE, 1);
    // 临时恢复 stderr 输出启动消息
    dup2(stderrBackup_, STDERR_FILENO);
    std::cout << "[Camera] 视频流已打开: " << sdpPath_ << std::endl;
    // 再次重定向 stderr（拑截后续解码错误）
    devNull = ::open("/dev/null", O_WRONLY);
    dup2(devNull, STDERR_FILENO);
    ::close(devNull);
    return true;
}

void Camera::close() {
    if (cap_.isOpened()) {
        cap_.release();
    }
    // 恢复 stderr
    if (stderrBackup_ >= 0) {
        dup2(stderrBackup_, STDERR_FILENO);
        ::close(stderrBackup_);
        stderrBackup_ = -1;
    }
    std::cout << "[Camera] 视频流已关闭" << std::endl;
}

bool Camera::isOpened() const {
    return cap_.isOpened();
}

bool Camera::getFrame(cv::Mat& frame) {
    if (!cap_.isOpened()) return false;
    return cap_.read(frame) && !frame.empty();
}
