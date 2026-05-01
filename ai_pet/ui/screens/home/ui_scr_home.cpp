#include "ui/screens/home/ui_scr_home.h"

#include "action/action_events.h"
#include "brain/brain_events.h"
#include "ui/common/ui_style.h"
#include "ui/managers/ui_manager.h"
#include "ui/screens/menu/ui_scr_menu.h"

#include <SDL2/SDL.h>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <mutex>
#include <cstdio>
#include <string_view>
#include <vector>

namespace {

constexpr int32_t kBubbleTextMaxWidth = 166;
constexpr int32_t kBubbleMaxWidth = 190;
constexpr int32_t kBubblePaddingX = 10;
constexpr int32_t kBubblePaddingY = 8;
constexpr int32_t kRowGap = 8;
constexpr int32_t kAvatarSize = 28;
constexpr int32_t kTimestampHeight = 24;
constexpr int32_t kTopPadding = 8;
constexpr int32_t kBottomPadding = 8;
constexpr int32_t kTagHeightExtra = 14;
constexpr int32_t kWheelScrollStep = 56;
constexpr int32_t kScrollBottomThreshold = 40;
constexpr size_t kChunkCharLimit = 72;
constexpr uint32_t kScrollActiveWindowMs = 900;

struct ChatMessage {
    std::string text;
    bool isUser = false;
    bool isProactive = false;
    std::string proactiveType;
    int32_t top = 0;
    int32_t textHeight = 0;
    int32_t bubbleWidth = 0;
    int32_t bubbleHeight = 0;
    int32_t height = 0;
    int32_t cacheWidth = 0;
    int32_t cacheHeight = 0;
    lv_draw_buf_t* cacheBuf = nullptr;
    lv_image_dsc_t cacheImage{};
};

lv_obj_t* g_screen = nullptr;
lv_obj_t* g_chat_panel = nullptr;
lv_obj_t* g_chat_canvas = nullptr;
lv_obj_t* g_cache_canvas = nullptr;
lv_obj_t* g_input = nullptr;
lv_obj_t* g_send_btn = nullptr;
lv_obj_t* g_menu_btn = nullptr;
lv_obj_t* g_emoji_btn = nullptr;
lv_obj_t* g_file_btn = nullptr;

std::vector<uint8_t> g_canvas_buffer;
std::vector<ChatMessage> g_messages;

int32_t g_canvas_width = 0;
int32_t g_canvas_height = 0;
int32_t g_content_height = 0;
int32_t g_scroll_offset = 0;
bool g_thinking = false;
uint32_t g_last_scroll_input_ms = 0;
bool g_bound = false;
std::mutex g_pending_mutex;
std::vector<ChatMessage> g_pending_messages;
bool g_pending_thinking = false;
bool g_pending_thinking_updated = false;

void draw_rect(lv_layer_t& layer, const lv_area_t& area, lv_color_t color, lv_opa_t opa, int32_t radius = 0);
void draw_text(lv_layer_t& layer, const lv_area_t& area, const char* text, lv_color_t color, const lv_font_t* font,
               lv_text_align_t align = LV_TEXT_ALIGN_LEFT);

std::string current_timestamp_label() {
    std::time_t now = std::time(nullptr);
    std::tm local_tm{};
#if defined(_WIN32)
    localtime_s(&local_tm, &now);
#else
    localtime_r(&now, &local_tm);
#endif

    char buffer[32];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "%d月%d日 %02d:%02d",
        local_tm.tm_mon + 1,
        local_tm.tm_mday,
        local_tm.tm_hour,
        local_tm.tm_min
    );
    return buffer;
}

void destroy_message_cache(ChatMessage& message) {
    if (message.cacheBuf) {
        lv_draw_buf_destroy(message.cacheBuf);
        message.cacheBuf = nullptr;
    }
    message.cacheWidth = 0;
    message.cacheHeight = 0;
}

void invalidate_all_message_cache() {
    for (auto& message : g_messages) {
        destroy_message_cache(message);
    }
}

size_t utf8_char_size(unsigned char c) {
    if ((c & 0x80u) == 0) return 1;
    if ((c & 0xE0u) == 0xC0u) return 2;
    if ((c & 0xF0u) == 0xE0u) return 3;
    if ((c & 0xF8u) == 0xF0u) return 4;
    return 1;
}

std::vector<std::string> split_long_message(const std::string& text) {
    std::vector<std::string> chunks;
    std::string current;
    size_t char_count = 0;

    auto flush = [&]() {
        if (!current.empty()) {
            chunks.push_back(current);
            current.clear();
            char_count = 0;
        }
    };

    for (size_t i = 0; i < text.size();) {
        const size_t cp_size = utf8_char_size(static_cast<unsigned char>(text[i]));
        const std::string_view cp(&text[i], std::min(cp_size, text.size() - i));
        const bool newline = cp == "\n";

        if (newline && char_count > kChunkCharLimit / 2) {
            flush();
            i += cp.size();
            continue;
        }

        current.append(cp.data(), cp.size());
        ++char_count;
        i += cp.size();

        if (char_count >= kChunkCharLimit) {
            flush();
        }
    }

    flush();
    if (chunks.empty()) {
        chunks.push_back(text);
    }
    return chunks;
}

int32_t viewport_width() {
    return g_canvas_width > 0 ? g_canvas_width : 1;
}

int32_t viewport_height() {
    return g_canvas_height > 0 ? g_canvas_height : 1;
}

int32_t max_scroll_offset() {
    return std::max(0, g_content_height - viewport_height());
}

void clamp_scroll_offset() {
    g_scroll_offset = std::clamp(g_scroll_offset, 0, max_scroll_offset());
}

bool should_stick_to_bottom() {
    return max_scroll_offset() - g_scroll_offset <= kScrollBottomThreshold;
}

lv_color_t proactive_bg(const std::string& type) {
    if (type == "check_in") return lv_color_hex(0xDBEAFE);
    if (type == "tease") return lv_color_hex(0xFCE7F3);
    if (type == "reminder") return lv_color_hex(0xFEE2E2);
    return lv_color_hex(0xFEF3C7);
}

lv_color_t proactive_text(const std::string& type) {
    if (type == "check_in") return lv_color_hex(0x1D4ED8);
    if (type == "tease") return lv_color_hex(0xBE185D);
    if (type == "reminder") return lv_color_hex(0xB91C1C);
    return lv_color_hex(0xB45309);
}

const char* proactive_label(const std::string& type) {
    if (type == "check_in") return "主动问候";
    if (type == "tease") return "主动逗弄";
    if (type == "reminder") return "主动提醒";
    return "主动关怀";
}

void ensure_canvas_buffer() {
    if (!g_chat_panel || !g_chat_canvas) {
        return;
    }

    const int32_t width = std::max(1, lv_obj_get_content_width(g_chat_panel));
    const int32_t height = std::max(1, lv_obj_get_content_height(g_chat_panel));
    if (width == g_canvas_width && height == g_canvas_height && !g_canvas_buffer.empty()) {
        return;
    }

    g_canvas_width = width;
    g_canvas_height = height;
    g_canvas_buffer.assign(static_cast<size_t>(width) * static_cast<size_t>(height) * 4u, 0);
    lv_canvas_set_buffer(g_chat_canvas, g_canvas_buffer.data(), width, height, LV_COLOR_FORMAT_ARGB8888);
    lv_obj_set_size(g_chat_canvas, width, height);
    invalidate_all_message_cache();
    clamp_scroll_offset();
}

void recalc_layout() {
    auto* manager = pet_ui::manager::UiManager::getInstance();
    const lv_font_t* font = manager->getBodyFont();

    int32_t y = kTimestampHeight + kTopPadding;
    for (auto& message : g_messages) {
        lv_point_t size{};
        lv_text_get_size(&size, message.text.c_str(), font, 0, 0, kBubbleTextMaxWidth, LV_TEXT_FLAG_NONE);
        message.textHeight = size.y;
        message.bubbleWidth = std::min(kBubbleMaxWidth, size.x + kBubblePaddingX * 2);
        message.bubbleHeight = size.y + kBubblePaddingY * 2;
        message.height = std::max(kAvatarSize, message.bubbleHeight);
        if (message.isProactive && !message.isUser) {
            message.height += kTagHeightExtra + 4;
        }
        message.top = y;
        y += message.height + kRowGap;
    }

    if (g_thinking) {
        y += 28;
    }

    g_content_height = y + kBottomPadding;
    clamp_scroll_offset();
}

void ensure_cache_canvas() {
    if (g_cache_canvas || !g_screen) {
        return;
    }
    g_cache_canvas = lv_canvas_create(g_screen);
    lv_obj_remove_style_all(g_cache_canvas);
    lv_obj_add_flag(g_cache_canvas, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(g_cache_canvas, 1, 1);
}

void render_message_cache(ChatMessage& message) {
    if (message.cacheBuf) {
        return;
    }

    ensure_cache_canvas();
    if (!g_cache_canvas) {
        return;
    }

    auto* manager = pet_ui::manager::UiManager::getInstance();
    const lv_font_t* font = manager->getBodyFont();

    const int32_t tagWidth = (message.isProactive && !message.isUser) ? 78 : 0;
    const int32_t tagHeight = (message.isProactive && !message.isUser) ? 22 : 0;
    const int32_t bubbleY = tagHeight > 0 ? tagHeight + 4 : 0;
    const int32_t cacheW = std::max(message.bubbleWidth, tagWidth);
    const int32_t cacheH = bubbleY + message.bubbleHeight;

    message.cacheBuf = lv_draw_buf_create(cacheW, cacheH, LV_COLOR_FORMAT_ARGB8888, 0);
    if (!message.cacheBuf) {
        return;
    }

    lv_canvas_set_draw_buf(g_cache_canvas, message.cacheBuf);
    lv_obj_set_size(g_cache_canvas, cacheW, cacheH);
    lv_canvas_fill_bg(g_cache_canvas, lv_color_black(), LV_OPA_TRANSP);

    lv_layer_t layer;
    lv_canvas_init_layer(g_cache_canvas, &layer);

    if (tagHeight > 0) {
        lv_area_t tagArea{0, 0, tagWidth - 1, tagHeight - 1};
        draw_rect(layer, tagArea, proactive_bg(message.proactiveType), LV_OPA_COVER, 0);
        draw_text(layer, tagArea, proactive_label(message.proactiveType), proactive_text(message.proactiveType), font);
    }

    lv_area_t bubbleArea{0, bubbleY, message.bubbleWidth - 1, bubbleY + message.bubbleHeight - 1};
    draw_rect(layer, bubbleArea, message.isUser ? lv_color_hex(0x95EC69) : lv_color_hex(0xFFFFFF), LV_OPA_COVER, 0);

    lv_area_t textArea{kBubblePaddingX, bubbleY + kBubblePaddingY,
                       message.bubbleWidth - kBubblePaddingX - 1,
                       bubbleY + message.bubbleHeight - kBubblePaddingY - 1};
    draw_text(layer, textArea, message.text.c_str(), lv_color_hex(0x111827), font);

    lv_canvas_finish_layer(g_cache_canvas, &layer);
    lv_draw_buf_to_image(message.cacheBuf, &message.cacheImage);
    message.cacheWidth = cacheW;
    message.cacheHeight = cacheH;
}

void draw_rect(lv_layer_t& layer, const lv_area_t& area, lv_color_t color, lv_opa_t opa, int32_t radius) {
    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    dsc.bg_opa = opa;
    dsc.bg_color = color;
    dsc.radius = radius;
    dsc.border_width = 0;
    dsc.shadow_width = 0;
    lv_draw_rect(&layer, &dsc, &area);
}

void draw_text(lv_layer_t& layer, const lv_area_t& area, const char* text, lv_color_t color, const lv_font_t* font, lv_text_align_t align) {
    lv_draw_label_dsc_t dsc;
    lv_draw_label_dsc_init(&dsc);
    dsc.text = text;
    dsc.color = color;
    dsc.font = const_cast<lv_font_t*>(font);
    dsc.align = align;
    lv_draw_label(&layer, &dsc, &area);
}

void redraw_chat_canvas() {
    if (!g_chat_canvas || !g_chat_panel) {
        return;
    }

    ensure_canvas_buffer();
    auto* manager = pet_ui::manager::UiManager::getInstance();
    const lv_font_t* font = manager->getBodyFont();

    lv_canvas_fill_bg(g_chat_canvas, lv_color_hex(0xEDEDED), LV_OPA_COVER);

    lv_layer_t layer;
    lv_canvas_init_layer(g_chat_canvas, &layer);

    const int32_t width = viewport_width();
    const int32_t height = viewport_height();

    const int32_t ts_y = -g_scroll_offset;
    if (ts_y + kTimestampHeight >= 0 && ts_y <= height) {
        lv_area_t ts_area{0, ts_y, width - 1, ts_y + kTimestampHeight - 1};
        const std::string timestamp = current_timestamp_label();
        draw_text(layer, ts_area, timestamp.c_str(), lv_color_hex(0x6B7280), font, LV_TEXT_ALIGN_CENTER);
    }

    for (const auto& message : g_messages) {
        const int32_t y = message.top - g_scroll_offset;
        const int32_t row_bottom = y + message.height;
        if (row_bottom < 0 || y > height) {
            continue;
        }

        int32_t cursor_y = y;
        const int32_t left_avatar_x = 0;
        const int32_t left_bubble_x = 36;

        render_message_cache(const_cast<ChatMessage&>(message));
        if (!message.cacheBuf) {
            continue;
        }

        lv_draw_image_dsc_t imgDsc;
        lv_draw_image_dsc_init(&imgDsc);
        imgDsc.src = &message.cacheImage;

        if (message.isUser) {
            const int32_t avatar_x = width - kAvatarSize - 2;
            const int32_t bubble_x = avatar_x - 6 - message.cacheWidth;
            lv_area_t avatar{avatar_x, y, avatar_x + kAvatarSize - 1, y + kAvatarSize - 1};
            draw_rect(layer, avatar, lv_color_hex(0x22C55E), LV_OPA_COVER, 0);
            draw_text(layer, avatar, "我", lv_color_hex(0xFFFFFF), font, LV_TEXT_ALIGN_CENTER);
            lv_area_t imgArea{bubble_x, cursor_y, bubble_x + message.cacheWidth - 1, cursor_y + message.cacheHeight - 1};
            lv_draw_image(&layer, &imgDsc, &imgArea);
        } else {
            lv_area_t avatar{left_avatar_x, y, left_avatar_x + kAvatarSize - 1, y + kAvatarSize - 1};
            draw_rect(layer, avatar, lv_color_hex(0x64748B), LV_OPA_COVER, 0);
            draw_text(layer, avatar, "宠", lv_color_hex(0xFFFFFF), font, LV_TEXT_ALIGN_CENTER);
            lv_area_t imgArea{left_bubble_x, cursor_y, left_bubble_x + message.cacheWidth - 1, cursor_y + message.cacheHeight - 1};
            lv_draw_image(&layer, &imgDsc, &imgArea);
        }
    }

    if (g_thinking) {
        const int32_t thinking_y = g_content_height - 24 - g_scroll_offset;
        if (thinking_y + 20 >= 0 && thinking_y <= height) {
            lv_area_t think_area{10, thinking_y, width - 10, thinking_y + 20};
            draw_text(layer, think_area, "思考中...", lv_color_hex(0x6B7280), font);
        }
    }

    if (g_content_height > height) {
        const int32_t track_x1 = width - 4;
        const int32_t thumb_h = std::max(24, height * height / std::max(height, g_content_height));
        const int32_t thumb_y = (height - thumb_h) * g_scroll_offset / std::max(1, max_scroll_offset());
        lv_area_t thumb{track_x1, thumb_y, width - 2, thumb_y + thumb_h};
        draw_rect(layer, thumb, lv_color_hex(0xBDBDBD), LV_OPA_70, 0);
    }

    lv_canvas_finish_layer(g_chat_canvas, &layer);
    lv_obj_invalidate(g_chat_canvas);
}

void append_message_chunks(const std::string& text, bool isUser, bool isProactive, const std::string& proactiveType) {
    const auto chunks = split_long_message(text);
    for (size_t i = 0; i < chunks.size(); ++i) {
        g_messages.push_back({chunks[i], isUser, isProactive && i == 0, proactiveType, 0, 0, 0, 0});
    }
}

void append_bubble(const std::string& text, bool isUser, bool isProactive, const std::string& proactiveType) {
    if (!g_chat_panel) {
        return;
    }

    const bool stick_bottom = should_stick_to_bottom();
    append_message_chunks(text, isUser, isProactive, proactiveType);
    recalc_layout();
    if (stick_bottom) {
        g_scroll_offset = max_scroll_offset();
    }
    redraw_chat_canvas();
}

void scroll_by(int32_t dy) {
    const int32_t old = g_scroll_offset;
    g_scroll_offset = std::clamp(g_scroll_offset + dy, 0, max_scroll_offset());
    if (g_scroll_offset != old) {
        g_last_scroll_input_ms = SDL_GetTicks();
        redraw_chat_canvas();
    }
}

void chat_panel_event_cb(lv_event_t* e) {
    const lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_PRESSING) {
        lv_point_t vect{};
        lv_indev_get_vect(lv_event_get_indev(e), &vect);
        scroll_by(-vect.y);
        return;
    }

    if (code == LV_EVENT_ROTARY) {
        const int32_t diff = lv_event_get_rotary_diff(e);
        if (diff != 0) {
            scroll_by(-diff * kWheelScrollStep);
        }
        return;
    }

    if (code == LV_EVENT_SIZE_CHANGED) {
        ensure_canvas_buffer();
        redraw_chat_canvas();
        return;
    }

    if (code == LV_EVENT_HOVER_OVER) {
        lv_group_focus_obj(g_chat_panel);
    }
}

void screen_event_cb(lv_event_t* e) {
    const lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* target = static_cast<lv_obj_t*>(lv_event_get_target(e));

    if (code == LV_EVENT_CLICKED && target == g_menu_btn) {
        pet_ui::screen::menu::load_screen();
        return;
    }

    if ((code == LV_EVENT_CLICKED && target == g_send_btn) || (code == LV_EVENT_READY && target == g_input)) {
        const char* text = lv_textarea_get_text(g_input);
        if (text && text[0] != '\0') {
            pet_ui::manager::UiManager::getInstance()->getEventBus()->emitTyped(brain::events::kUserInputText, brain::events::UserTextInputEvent{text});
            lv_textarea_set_text(g_input, "");
        }
    }
}

void screen_cleanup_cb(lv_event_t*) {
    g_screen = nullptr;
    g_chat_panel = nullptr;
    g_chat_canvas = nullptr;
    g_cache_canvas = nullptr;
    g_input = nullptr;
    g_send_btn = nullptr;
    g_menu_btn = nullptr;
    g_emoji_btn = nullptr;
    g_file_btn = nullptr;
    g_canvas_buffer.clear();
    invalidate_all_message_cache();
    g_messages.clear();
    g_canvas_width = 0;
    g_canvas_height = 0;
    g_content_height = 0;
    g_scroll_offset = 0;
    g_thinking = false;
    g_last_scroll_input_ms = 0;
}

void create_screen() {
    auto* manager = pet_ui::manager::UiManager::getInstance();

    g_screen = lv_obj_create(manager->getContentHost());
    pet_ui::style::style_screen_root(g_screen);
    manager->registerScreen(pet_ui::manager::ScreenType::Home, &g_screen);
    lv_obj_add_event_cb(g_screen, screen_cleanup_cb, LV_EVENT_DELETE, nullptr);

    lv_obj_set_style_pad_all(g_screen, 0, 0);
    lv_obj_set_style_pad_row(g_screen, 0, 0);

    g_chat_panel = lv_obj_create(g_screen);
    lv_obj_remove_style_all(g_chat_panel);
    lv_obj_set_width(g_chat_panel, lv_pct(100));
    lv_obj_set_flex_grow(g_chat_panel, 1);
    lv_obj_set_style_bg_color(g_chat_panel, lv_color_hex(0xEDEDED), 0);
    lv_obj_set_style_bg_opa(g_chat_panel, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(g_chat_panel, 0, 0);
    lv_obj_clear_flag(g_chat_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(g_chat_panel, chat_panel_event_cb, LV_EVENT_PRESSING, nullptr);
    lv_obj_add_event_cb(g_chat_panel, chat_panel_event_cb, LV_EVENT_ROTARY, nullptr);
    lv_obj_add_event_cb(g_chat_panel, chat_panel_event_cb, LV_EVENT_SIZE_CHANGED, nullptr);
    lv_obj_add_event_cb(g_chat_panel, chat_panel_event_cb, LV_EVENT_HOVER_OVER, nullptr);

    g_chat_canvas = lv_canvas_create(g_chat_panel);
    lv_obj_remove_style_all(g_chat_canvas);
    lv_obj_set_size(g_chat_canvas, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(g_chat_canvas, LV_OPA_TRANSP, 0);
    lv_obj_center(g_chat_canvas);
    lv_obj_add_flag(g_chat_canvas, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_flag(g_chat_canvas, LV_OBJ_FLAG_GESTURE_BUBBLE);

    ensure_cache_canvas();

    lv_obj_t* input_area = lv_obj_create(g_screen);
    lv_obj_set_width(input_area, lv_pct(100));
    lv_obj_set_height(input_area, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(input_area, lv_color_hex(0xF7F7F7), 0);
    lv_obj_set_style_bg_opa(input_area, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(input_area, 0, 0);
    lv_obj_set_style_radius(input_area, 0, 0);
    lv_obj_set_style_pad_left(input_area, 10, 0);
    lv_obj_set_style_pad_right(input_area, 10, 0);
    lv_obj_set_style_pad_top(input_area, 6, 0);
    lv_obj_set_style_pad_bottom(input_area, 8, 0);
    lv_obj_set_style_pad_row(input_area, 6, 0);
    lv_obj_set_flex_flow(input_area, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(input_area, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* toolbar = lv_obj_create(input_area);
    lv_obj_remove_style_all(toolbar);
    lv_obj_set_width(toolbar, lv_pct(100));
    lv_obj_set_height(toolbar, 28);
    lv_obj_set_style_bg_opa(toolbar, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(toolbar, 0, 0);
    lv_obj_set_style_pad_all(toolbar, 0, 0);
    lv_obj_set_flex_flow(toolbar, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(toolbar, 4, 0);
    lv_obj_clear_flag(toolbar, LV_OBJ_FLAG_SCROLLABLE);

    g_emoji_btn = lv_button_create(toolbar);
    lv_obj_set_size(g_emoji_btn, 28, 28);
    lv_obj_set_style_bg_opa(g_emoji_btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(g_emoji_btn, 0, 0);
    lv_obj_t* emoji_label = lv_label_create(g_emoji_btn);
    lv_label_set_text(emoji_label, "+");
    lv_obj_center(emoji_label);

    g_file_btn = lv_button_create(toolbar);
    lv_obj_set_size(g_file_btn, 28, 28);
    lv_obj_set_style_bg_opa(g_file_btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(g_file_btn, 0, 0);
    lv_obj_t* file_label = lv_label_create(g_file_btn);
    lv_label_set_text(file_label, LV_SYMBOL_DIRECTORY);
    lv_obj_center(file_label);

    lv_obj_t* input_row = lv_obj_create(input_area);
    lv_obj_remove_style_all(input_row);
    lv_obj_set_width(input_row, lv_pct(100));
    lv_obj_set_height(input_row, 40);
    lv_obj_set_style_bg_opa(input_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(input_row, 0, 0);
    lv_obj_set_style_pad_all(input_row, 0, 0);
    lv_obj_set_flex_flow(input_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(input_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(input_row, 6, 0);
    lv_obj_clear_flag(input_row, LV_OBJ_FLAG_SCROLLABLE);

    g_menu_btn = lv_button_create(input_row);
    lv_obj_set_size(g_menu_btn, 36, 36);
    pet_ui::style::style_button(g_menu_btn, lv_color_hex(0xD4D4D8));
    lv_obj_set_style_radius(g_menu_btn, 18, 0);
    lv_obj_t* menu_icon = lv_label_create(g_menu_btn);
    lv_label_set_text(menu_icon, LV_SYMBOL_LIST);
    lv_obj_center(menu_icon);
    lv_obj_add_event_cb(g_menu_btn, screen_event_cb, LV_EVENT_CLICKED, nullptr);

    g_input = lv_textarea_create(input_row);
    lv_obj_set_height(g_input, 36);
    lv_obj_set_flex_grow(g_input, 1);
    lv_textarea_set_placeholder_text(g_input, "输入消息...");
    lv_textarea_set_one_line(g_input, true);
    lv_obj_set_style_bg_color(g_input, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_color(g_input, lv_color_hex(0x111827), 0);
    lv_obj_set_style_border_width(g_input, 0, 0);
    lv_obj_set_style_radius(g_input, 4, 0);
    lv_obj_set_style_pad_left(g_input, 10, 0);
    lv_obj_set_style_pad_right(g_input, 10, 0);
    lv_obj_set_style_text_font(g_input, manager->getBodyFont(), 0);
    lv_obj_add_event_cb(g_input, screen_event_cb, LV_EVENT_READY, nullptr);

    g_send_btn = lv_button_create(input_row);
    lv_obj_set_size(g_send_btn, 54, 36);
    lv_obj_set_style_bg_color(g_send_btn, lv_color_hex(0x07C160), 0);
    lv_obj_set_style_bg_opa(g_send_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(g_send_btn, 0, 0);
    lv_obj_set_style_radius(g_send_btn, 4, 0);
    lv_obj_set_style_pad_all(g_send_btn, 0, 0);
    lv_obj_t* send_label = lv_label_create(g_send_btn);
    lv_label_set_text(send_label, "发送");
    lv_obj_set_style_text_color(send_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(send_label, manager->getBodyFont(), 0);
    lv_obj_center(send_label);
    lv_obj_add_event_cb(g_send_btn, screen_event_cb, LV_EVENT_CLICKED, nullptr);

    lv_obj_update_layout(g_screen);
    ensure_canvas_buffer();
    append_bubble("陪伴主界面...情绪、关系和记忆会驱动界面变化。", false, false, std::string());
}

}  // namespace

namespace pet_ui::screen::home {

void bind_event_bus(AppEventBus& eventBus) {
    if (g_bound) {
        return;
    }
    g_bound = true;

    eventBus.subscribeTyped<brain::events::UserTextInputEvent>(brain::events::kUserInputText, [](const brain::events::UserTextInputEvent& event) {
        std::lock_guard<std::mutex> lock(g_pending_mutex);
        g_pending_messages.push_back({event.text, true, false, std::string()});
    });
    eventBus.subscribeTyped<action::events::ChatReplyEvent>(action::events::kChatReply, [](const action::events::ChatReplyEvent& event) {
        std::lock_guard<std::mutex> lock(g_pending_mutex);
        g_pending_messages.push_back({event.text, false, false, std::string()});
    });
    eventBus.subscribeTyped<action::events::ThinkingEvent>(action::events::kChatThinkingStart, [](const action::events::ThinkingEvent&) {
        std::lock_guard<std::mutex> lock(g_pending_mutex);
        g_pending_thinking = true;
        g_pending_thinking_updated = true;
    });
    eventBus.subscribeTyped<action::events::ThinkingEvent>(action::events::kChatThinkingEnd, [](const action::events::ThinkingEvent&) {
        std::lock_guard<std::mutex> lock(g_pending_mutex);
        g_pending_thinking = false;
        g_pending_thinking_updated = true;
    });
    eventBus.subscribeTyped<action::events::ProactiveBehaviorEvent>(action::events::kBehaviorProactiveCare, [](const action::events::ProactiveBehaviorEvent& event) {
        std::lock_guard<std::mutex> lock(g_pending_mutex);
        g_pending_messages.push_back({event.text, false, true, event.type});
    });
    eventBus.subscribeTyped<action::events::ProactiveBehaviorEvent>(action::events::kBehaviorProactiveCheckIn, [](const action::events::ProactiveBehaviorEvent& event) {
        std::lock_guard<std::mutex> lock(g_pending_mutex);
        g_pending_messages.push_back({event.text, false, true, event.type});
    });
    eventBus.subscribeTyped<action::events::ProactiveBehaviorEvent>(action::events::kBehaviorProactiveTease, [](const action::events::ProactiveBehaviorEvent& event) {
        std::lock_guard<std::mutex> lock(g_pending_mutex);
        g_pending_messages.push_back({event.text, false, true, event.type});
    });
    eventBus.subscribeTyped<action::events::ProactiveBehaviorEvent>(action::events::kBehaviorProactiveReminder, [](const action::events::ProactiveBehaviorEvent& event) {
        std::lock_guard<std::mutex> lock(g_pending_mutex);
        g_pending_messages.push_back({event.text, false, true, event.type});
    });
}

void process_pending() {
    std::vector<ChatMessage> messages;
    bool thinkingUpdated = false;
    bool thinking = false;
    {
        std::lock_guard<std::mutex> lock(g_pending_mutex);
        messages.swap(g_pending_messages);
        thinkingUpdated = g_pending_thinking_updated;
        thinking = g_pending_thinking;
        g_pending_thinking_updated = false;
    }

    for (const auto& message : messages) {
        append_message(message.text, message.isUser, message.isProactive, message.proactiveType);
    }
    if (thinkingUpdated) {
        set_thinking(thinking);
    }
}

void load_screen() {
    auto* manager = pet_ui::manager::UiManager::getInstance();
    if (!g_screen) {
        create_screen();
    }

    manager->resetKeypadGroup();
    manager->addObjToGroup(g_chat_panel);
    manager->addObjToGroup(g_menu_btn);
    manager->addObjToGroup(g_input);
    manager->addObjToGroup(g_send_btn);
    lv_group_focus_obj(g_chat_panel);
    manager->loadContentScreen(pet_ui::manager::ScreenType::Home, g_screen);
    redraw_chat_canvas();
}

void append_message(const std::string& text, bool isUser, bool isProactive, const std::string& proactiveType) {
    if (!g_screen) {
        load_screen();
    }
    append_bubble(text, isUser, isProactive, proactiveType);
}

void set_thinking(bool thinking) {
    g_thinking = thinking;
    recalc_layout();
    if (thinking && should_stick_to_bottom()) {
        g_scroll_offset = max_scroll_offset();
    }
    redraw_chat_canvas();
}

void set_persona_name(const std::string& name) {
    (void)name;
}

bool is_scroll_active() {
    if (!g_screen) {
        return false;
    }
    const uint32_t now = SDL_GetTicks();
    return g_last_scroll_input_ms != 0 && now - g_last_scroll_input_ms <= kScrollActiveWindowMs;
}

}  // namespace pet_ui::screen::home
