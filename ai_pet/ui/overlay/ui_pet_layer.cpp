#include "ui/overlay/ui_pet_layer.h"
#include "action/action_events.h"
#include "ui/assets/pet/white/happy/dog_white_happy.h"
#include "ui/assets/pet/white/idle/dog_white_idle.h"
#include "ui/assets/pet/white/sad/dog_white_sad.h"
#include "ui/managers/ui_manager.h"
#include "lvgl.h"
#include <SDL2/SDL.h>
#include <cstring>
#include <mutex>
#include <vector>

#ifndef APP_ASSET_DIR
#define APP_ASSET_DIR "ui/assets"
#endif

namespace {

lv_obj_t* g_root = nullptr;
lv_obj_t* g_pet_image = nullptr;
lv_obj_t* g_camera_canvas = nullptr;
std::vector<unsigned char> g_camera_buffer;
constexpr int kCamW = 96;
constexpr int kCamH = 72;
bool g_perf_mode = false;
bool g_bound = false;
uint32_t g_last_camera_draw_ms = 0;
std::mutex g_pending_mutex;
std::string g_pending_emotion;
bool g_pending_emotion_updated = false;
enum class PetMood { Idle, Happy, Sad };
PetMood g_current_mood = PetMood::Idle;
const void* kIdleFrames[] = {&dog_white_idle_0, &dog_white_idle_1, &dog_white_idle_2, &dog_white_idle_3};
const void* kHappyFrames[] = {&dog_white_happy_0, &dog_white_happy_1, &dog_white_happy_2, &dog_white_happy_3, &dog_white_happy_4};
const void* kSadFrames[] = {&dog_white_sad_0, &dog_white_sad_1, &dog_white_sad_2, &dog_white_sad_3};

void set_pet_anim(const void* const* frames, uint32_t count, uint32_t duration) {
    if (!g_pet_image) {
        return;
    }
    lv_animimg_set_src(g_pet_image, const_cast<const void**>(frames), count);
    lv_animimg_set_duration(g_pet_image, duration);
    lv_animimg_set_repeat_count(g_pet_image, LV_ANIM_REPEAT_INFINITE);
    lv_animimg_start(g_pet_image);
}

uint32_t mood_duration(PetMood mood) {
    switch (mood) {
        case PetMood::Happy:
            return g_perf_mode ? 1800 : 720;
        case PetMood::Sad:
            return g_perf_mode ? 2200 : 920;
        case PetMood::Idle:
        default:
            return g_perf_mode ? 2000 : 900;
    }
}

void apply_pet_anim() {
    switch (g_current_mood) {
        case PetMood::Happy:
            set_pet_anim(kHappyFrames, 5, mood_duration(PetMood::Happy));
            return;
        case PetMood::Sad:
            set_pet_anim(kSadFrames, 4, mood_duration(PetMood::Sad));
            return;
        case PetMood::Idle:
        default:
            set_pet_anim(kIdleFrames, 4, mood_duration(PetMood::Idle));
            return;
    }
}

}  // namespace

namespace pet_ui::overlay::pet_layer {

void create() {
    auto* manager = pet_ui::manager::UiManager::getInstance();
    lv_obj_t* parent = manager->getLeftPanel();

    g_root = lv_obj_create(parent);
    lv_obj_set_size(g_root, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(g_root, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(g_root, 0, 0);
    lv_obj_set_style_pad_all(g_root, 0, 0);
    lv_obj_clear_flag(g_root, LV_OBJ_FLAG_SCROLLABLE);
    manager->registerScreen(
        pet_ui::manager::ScreenType::PetOverlay,
        &g_root,
        pet_ui::manager::UiScreenLifecycleManager::ScreenCategory::OverlayLayer
    );

    lv_obj_t* bg = lv_image_create(g_root);
    lv_image_set_src(bg, "/" APP_ASSET_DIR "/backgrounds/left/default.png");
    lv_obj_set_size(bg, lv_pct(100), lv_pct(100));
    lv_obj_set_pos(bg, 0, 0);
    lv_obj_add_flag(bg, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_move_to_index(bg, 0);

    g_pet_image = lv_animimg_create(g_root);
    set_pet_anim(kIdleFrames, 4, 900);
    lv_obj_align(g_pet_image, LV_ALIGN_CENTER, 0, 20);

    g_camera_buffer.resize(kCamW * kCamH * 4, 0);
    g_camera_canvas = lv_canvas_create(g_root);
    lv_canvas_set_buffer(g_camera_canvas, g_camera_buffer.data(), kCamW, kCamH, LV_COLOR_FORMAT_ARGB8888);
    lv_obj_set_style_bg_color(g_camera_canvas, lv_color_hex(0x111827), 0);
    lv_obj_set_style_bg_opa(g_camera_canvas, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(g_camera_canvas, 10, 0);
    lv_obj_set_style_clip_corner(g_camera_canvas, true, 0);
    lv_obj_set_style_border_width(g_camera_canvas, 1, 0);
    lv_obj_set_style_border_color(g_camera_canvas, lv_color_hex(0xE9D8A6), 0);
    lv_obj_set_style_shadow_width(g_camera_canvas, 12, 0);
    lv_obj_set_style_shadow_opa(g_camera_canvas, LV_OPA_20, 0);
    lv_obj_set_style_shadow_color(g_camera_canvas, lv_color_hex(0x8B6F47), 0);
    lv_obj_align(g_camera_canvas, LV_ALIGN_TOP_MID, 0, 42);
    lv_obj_move_foreground(g_camera_canvas);
}

void bind_event_bus(AppEventBus& eventBus) {
    if (g_bound) {
        return;
    }
    g_bound = true;

    eventBus.subscribeTyped<action::events::PetStateEvent>(action::events::kPetEmotion, [](const action::events::PetStateEvent& event) {
        std::lock_guard<std::mutex> lock(g_pending_mutex);
        g_pending_emotion = event.value;
        g_pending_emotion_updated = true;
    });
}

void process_pending() {
    std::string emotion;
    bool updated = false;
    {
        std::lock_guard<std::mutex> lock(g_pending_mutex);
        emotion = g_pending_emotion;
        updated = g_pending_emotion_updated;
        g_pending_emotion_updated = false;
    }
    if (updated) {
        update_emotion(emotion);
    }
}

void update_camera_frame(const cv::Mat& frame) {
    if (!g_camera_canvas || frame.empty()) {
        return;
    }
    const uint32_t now = SDL_GetTicks();
    const uint32_t min_interval = g_perf_mode ? 160 : 66;
    if (g_last_camera_draw_ms != 0 && now - g_last_camera_draw_ms < min_interval) {
        return;
    }
    g_last_camera_draw_ms = now;
    cv::Mat resized;
    cv::resize(frame, resized, cv::Size(kCamW, kCamH));
    cv::Mat bgra;
    cv::cvtColor(resized, bgra, cv::COLOR_BGR2BGRA);
    std::memcpy(g_camera_buffer.data(), bgra.data, kCamW * kCamH * 4);
    lv_obj_invalidate(g_camera_canvas);
}

void update_emotion(const std::string& emotion) {
    if (emotion.find("happy") != std::string::npos || emotion.find("开心") != std::string::npos) {
        g_current_mood = PetMood::Happy;
        apply_pet_anim();
        return;
    }
    if (emotion.find("sad") != std::string::npos || emotion.find("难过") != std::string::npos) {
        g_current_mood = PetMood::Sad;
        apply_pet_anim();
        return;
    }
    g_current_mood = PetMood::Idle;
    apply_pet_anim();
}

void set_performance_mode(bool enabled) {
    if (g_perf_mode == enabled) {
        return;
    }
    g_perf_mode = enabled;
    apply_pet_anim();
}

}  // namespace pet_ui::overlay::pet_layer
