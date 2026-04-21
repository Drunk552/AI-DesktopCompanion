#include "app/mode_runner_camera.h"
#include "app/runtime_context.h"
#include "app/module_registry.h"
#include "brain/brain_controller.h"
#include "brain/brain_events.h"
#include "perception/camera.h"
#include "perception/vision_pipeline.h"
#include "shared/logger/logger.h"
#include <opencv2/opencv.hpp>

CameraModeRunner::CameraModeRunner(RuntimeContext& context)
    : context_(context) {}

void CameraModeRunner::run() {
    LOGI("Application", "启动摄像头测试模式");
    auto runtime = context_.modules.runtimeServices();
    context_.brainController.setActiveMode("camera");
    context_.brainController.init(context_.eventBus, {});

    if (!context_.initVision()) {
        LOGE("Application", "视觉模块初始化失败");
        return;
    }

    if (!runtime.camera.open()) {
        LOGE("Application", "无法打开摄像头，请检查 /tmp/yuyv.sdp 是否存在");
        return;
    }

    LOGI("Application", "摄像头已打开，按 'q' 退出");

    cv::Mat frame;
    while (true) {
        if (runtime.camera.getFrame(frame)) {
            VisionResult vr = runtime.visionPipeline.process(frame);
            if (vr.faceDetected && !vr.emotion.empty()) {
                context_.eventBus.emitTyped(brain::events::kPerceptionEmotionDetected, brain::events::PerceptionEmotionEvent{vr.emotion});
            }
            cv::imshow("Camera - Face & Emotion", vr.annotatedFrame);
        }
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    cv::destroyAllWindows();
    runtime.camera.close();
}
