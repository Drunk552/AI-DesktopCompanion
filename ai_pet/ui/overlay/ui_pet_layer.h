#pragma once

#include "app/app_event_bus.h"
#include <opencv2/opencv.hpp>
#include <string>

namespace pet_ui::overlay::pet_layer {

void create();
void bind_event_bus(AppEventBus& eventBus);
void process_pending();
void update_camera_frame(const cv::Mat& frame);
void update_emotion(const std::string& emotion);
void set_performance_mode(bool enabled);

}  // namespace pet_ui::overlay::pet_layer
