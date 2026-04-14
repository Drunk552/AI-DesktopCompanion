#include "vision/camera.h"
#include "logger/logger.h"
#include <fstream>
#include <sstream>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>

extern "C" {
#include <libavutil/log.h>
}

static void silentLogCallback(void*, int, const char*, va_list) {}

static void fixSdpPort(const std::string& sdpPath, int port) {
    std::ifstream in(sdpPath);
    if (!in.is_open()) return;

    std::stringstream buf;
    buf << in.rdbuf();
    in.close();

    std::string content = buf.str();
    std::string search = "m=video 0 ";
    std::string replace = "m=video " + std::to_string(port) + " ";
    auto pos = content.find(search);
    if (pos != std::string::npos) {
        content.replace(pos, search.length(), replace);
        std::ofstream out(sdpPath);
        out << content;
        out.close();
        LOGI("Camera", "已修正 SDP 端口: 0 -> " + std::to_string(port));
    }
}

Camera::Camera(const std::string& sdpPath)
    : sdpPath_(sdpPath) {}

Camera::~Camera() {
    close();
}

bool Camera::open() {
    av_log_set_level(AV_LOG_QUIET);
    av_log_set_callback(silentLogCallback);

    fixSdpPort(sdpPath_, 5004);

    stderrBackup_ = dup(STDERR_FILENO);
    int devNull = ::open("/dev/null", O_WRONLY);
    dup2(devNull, STDERR_FILENO);
    ::close(devNull);

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
        dup2(stderrBackup_, STDERR_FILENO);
        ::close(stderrBackup_);
        stderrBackup_ = -1;
        LOGE("Camera", "无法打开视频流: " + sdpPath_);
        LOGE("Camera", "请检查: 1) SDP文件是否存在  2) Windows端是否在推流  3) 网络是否连通");
        return false;
    }
    cap_.set(cv::CAP_PROP_BUFFERSIZE, 1);
    dup2(stderrBackup_, STDERR_FILENO);
    LOGI("Camera", "视频流已打开: " + sdpPath_);
    devNull = ::open("/dev/null", O_WRONLY);
    dup2(devNull, STDERR_FILENO);
    ::close(devNull);
    return true;
}

void Camera::close() {
    if (cap_.isOpened()) {
        cap_.release();
    }
    if (stderrBackup_ >= 0) {
        dup2(stderrBackup_, STDERR_FILENO);
        ::close(stderrBackup_);
        stderrBackup_ = -1;
    }
    LOGI("Camera", "视频流已关闭");
}

bool Camera::isOpened() const {
    return cap_.isOpened();
}

bool Camera::getFrame(cv::Mat& frame) {
    if (!cap_.isOpened()) return false;
    return cap_.read(frame) && !frame.empty();
}
