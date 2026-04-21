#include "ui/ui_app.h"
#include "shared/logger/logger.h"
#include "ui/controller/ui_controller.h"
#include "ui/managers/ui_manager.h"
#include "ui/overlay/ui_notification.h"
#include "ui/overlay/ui_pet_layer.h"
#include "ui/overlay/ui_status_bar.h"
#include "ui/screens/home/ui_scr_home.h"
#include "ui/screens/interaction/ui_interaction.h"
#include <SDL2/SDL.h>
#include <fstream>

namespace {

bool g_initialized = false;
lv_display_t* g_display = nullptr;
lv_group_t* g_group = nullptr;
lv_font_t* g_font_body = nullptr;
lv_font_t* g_font_title = nullptr;
std::vector<unsigned char> g_font_data;
uint32_t g_last_tick_ms = 0;

void load_fonts() {
    const char* fontPath = "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc";
    std::ifstream fontFile(fontPath, std::ios::binary | std::ios::ate);
    if (fontFile.is_open()) {
        size_t fileSize = fontFile.tellg();
        fontFile.seekg(0, std::ios::beg);
        g_font_data.resize(fileSize);
        fontFile.read(reinterpret_cast<char*>(g_font_data.data()), fileSize);
        g_font_body = lv_tiny_ttf_create_data(g_font_data.data(), g_font_data.size(), 14);
        g_font_title = lv_tiny_ttf_create_data(g_font_data.data(), g_font_data.size(), 16);
    }
    if (!g_font_body || !g_font_title) {
        g_font_body = const_cast<lv_font_t*>(&lv_font_montserrat_14);
        g_font_title = const_cast<lv_font_t*>(&lv_font_montserrat_16);
    }
}

}  // namespace

namespace pet_ui::app {

bool init(int width, int height, AppEventBus* eventBus) {
    if (g_initialized) {
        return true;
    }

    lv_init();
    g_display = lv_sdl_window_create(width, height);
    if (!g_display) {
        LOGE("UI", "无法创建 SDL 窗口");
        return false;
    }

    lv_sdl_mouse_create();
    lv_indev_t* keyboard = lv_sdl_keyboard_create();
    g_group = lv_group_create();
    lv_group_set_default(g_group);
    if (keyboard) {
        lv_indev_set_group(keyboard, g_group);
    }

    load_fonts();

    pet_ui::manager::UiManager::getInstance()->init(width, height, g_group, g_font_body, g_font_title, eventBus);
    pet_ui::screen::home::bind_event_bus(*eventBus);
    pet_ui::overlay::pet_layer::create();
    pet_ui::overlay::pet_layer::bind_event_bus(*eventBus);
    pet_ui::overlay::status_bar::create();
    pet_ui::overlay::status_bar::bind_event_bus(*eventBus);
    pet_ui::overlay::notification::create();
    pet_ui::screen::interaction::bind_event_bus(*eventBus);
    pet_ui::controller::UiController::getInstance()->init(eventBus);
    pet_ui::screen::home::load_screen();

    g_last_tick_ms = SDL_GetTicks();
    g_initialized = true;
    LOGI("UI", "按模块重构后的 UI 已初始化");
    return true;
}

bool tick() {
    if (!g_initialized) {
        return false;
    }

    const uint32_t now = SDL_GetTicks();
    const uint32_t delta = g_last_tick_ms == 0 ? 16 : (now > g_last_tick_ms ? now - g_last_tick_ms : 1);
    g_last_tick_ms = now;

    pet_ui::screen::home::process_pending();
    pet_ui::overlay::pet_layer::process_pending();
    pet_ui::overlay::status_bar::process_pending();
    pet_ui::screen::interaction::process_pending();
    pet_ui::controller::UiController::getInstance()->processPending();
    pet_ui::overlay::notification::tick();
    lv_tick_inc(delta);
    uint32_t waitMs = lv_timer_handler();
    SDL_Delay(waitMs < 1 ? 1 : waitMs);
    return true;
}

void shutdown() {
    if (!g_initialized) {
        return;
    }
    g_initialized = false;
    g_last_tick_ms = 0;
    LOGI("UI", "UI 已关闭");
}

}  // namespace pet_ui::app
