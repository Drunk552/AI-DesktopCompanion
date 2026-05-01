#include "ui/overlay/ui_status_bar.h"
#include "action/action_events.h"
#include "brain/brain_events.h"
#include "ui/common/ui_widgets.h"
#include "ui/managers/ui_manager.h"
#include <mutex>

namespace {

lv_obj_t* g_pet_root = nullptr;
lv_obj_t* g_persona_root = nullptr;
lv_obj_t* g_pet_emotion = nullptr;
lv_obj_t* g_pet_affection = nullptr;
lv_obj_t* g_pet_relationship = nullptr;
lv_obj_t* g_persona_name = nullptr;
bool g_bound = false;
std::mutex g_pending_mutex;
brain::BrainState g_pending_state;
bool g_pending_state_updated = false;
std::string g_pending_affection;
bool g_pending_affection_updated = false;
std::string g_pending_relationship;
bool g_pending_relationship_updated = false;

void set_label(lv_obj_t* label, const std::string& text) {
    if (label) {
        lv_label_set_text(label, text.c_str());
    }
}

}  // namespace

namespace pet_ui::overlay::status_bar {

void create() {
    auto* manager = pet_ui::manager::UiManager::getInstance();
    lv_font_t* font = manager->getBodyFont();
    lv_font_t* titleFont = manager->getTitleFont();

    lv_obj_t* left_panel = manager->getLeftPanel();
    lv_obj_t* right_host = manager->getStatusHost();

    g_pet_root = lv_obj_create(left_panel);
    lv_obj_set_size(g_pet_root, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(g_pet_root, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(g_pet_root, 0, 0);
    lv_obj_set_style_pad_left(g_pet_root, 14, 0);
    lv_obj_set_style_pad_right(g_pet_root, 14, 0);
    lv_obj_set_style_pad_top(g_pet_root, 0, 0);
    lv_obj_set_style_pad_bottom(g_pet_root, 12, 0);
    lv_obj_set_style_pad_column(g_pet_root, 6, 0);
    lv_obj_set_style_pad_row(g_pet_root, 6, 0);
    lv_obj_set_flex_flow(g_pet_root, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(g_pet_root, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_align(g_pet_root, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_flag(g_pet_root, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_clear_flag(g_pet_root, LV_OBJ_FLAG_SCROLLABLE);

    g_persona_root = lv_obj_create(right_host);
    lv_obj_set_size(g_persona_root, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(g_persona_root, lv_color_hex(0xF7F7F7), 0);
    lv_obj_set_style_bg_opa(g_persona_root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(g_persona_root, 1, 0);
    lv_obj_set_style_border_color(g_persona_root, lv_color_hex(0xD1D5DB), 0);
    lv_obj_set_style_border_side(g_persona_root, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_pad_left(g_persona_root, 12, 0);
    lv_obj_set_style_pad_right(g_persona_root, 12, 0);
    lv_obj_set_style_pad_top(g_persona_root, 3, 0);
    lv_obj_set_style_pad_bottom(g_persona_root, 3, 0);
    lv_obj_set_style_radius(g_persona_root, 0, 0);
    lv_obj_clear_flag(g_persona_root, LV_OBJ_FLAG_SCROLLABLE);
    manager->registerScreen(
        pet_ui::manager::ScreenType::StatusOverlay,
        &g_persona_root,
        pet_ui::manager::UiScreenLifecycleManager::ScreenCategory::OverlayLayer
    );

    g_pet_emotion = pet_ui::widgets::create_status_chip(g_pet_root, "开心", lv_color_hex(0x8FE3B0), font);
    g_pet_affection = pet_ui::widgets::create_status_chip(g_pet_root, "亲密度 48", lv_color_hex(0xF5D77A), font);
    g_pet_relationship = pet_ui::widgets::create_status_chip(g_pet_root, "来密", lv_color_hex(0xA5B6D3), font);

    g_persona_name = lv_label_create(g_persona_root);
    lv_label_set_text(g_persona_name, "紫金公主");
    lv_obj_set_style_text_color(g_persona_name, lv_color_hex(0x111827), 0);
    if (titleFont) {
        lv_obj_set_style_text_font(g_persona_name, titleFont, 0);
    } else if (font) {
        lv_obj_set_style_text_font(g_persona_name, font, 0);
    }
    lv_obj_align(g_persona_name, LV_ALIGN_CENTER, 0, 0);
}

void bind_event_bus(AppEventBus& eventBus) {
    if (g_bound) {
        return;
    }
    g_bound = true;

    eventBus.subscribeTyped<brain::BrainStateChangedEvent>(brain::events::kBrainStateChanged, [](const brain::BrainStateChangedEvent& event) {
        std::lock_guard<std::mutex> lock(g_pending_mutex);
        g_pending_state = event.state;
        g_pending_state_updated = true;
    });
    eventBus.subscribeTyped<action::events::PetStateEvent>(action::events::kPetAffection, [](const action::events::PetStateEvent& event) {
        std::lock_guard<std::mutex> lock(g_pending_mutex);
        g_pending_affection = event.value;
        g_pending_affection_updated = true;
    });
    eventBus.subscribeTyped<action::events::PetStateEvent>(action::events::kPetRelationship, [](const action::events::PetStateEvent& event) {
        std::lock_guard<std::mutex> lock(g_pending_mutex);
        g_pending_relationship = event.value;
        g_pending_relationship_updated = true;
    });
}

void process_pending() {
    brain::BrainState state;
    bool stateUpdated = false;
    std::string affection;
    bool affectionUpdated = false;
    std::string relationship;
    bool relationshipUpdated = false;
    {
        std::lock_guard<std::mutex> lock(g_pending_mutex);
        state = g_pending_state;
        stateUpdated = g_pending_state_updated;
        affection = g_pending_affection;
        affectionUpdated = g_pending_affection_updated;
        relationship = g_pending_relationship;
        relationshipUpdated = g_pending_relationship_updated;
        g_pending_state_updated = false;
        g_pending_affection_updated = false;
        g_pending_relationship_updated = false;
    }

    if (stateUpdated) {
        apply_brain_state(state);
    }
    if (affectionUpdated) {
        update_affection(affection);
    }
    if (relationshipUpdated) {
        update_relationship(relationship);
    }
}

void update_persona_name(const std::string& name) {
    set_label(g_persona_name, name.empty() ? "紫金公主" : name);
}

void update_emotion(const std::string& emotion) {
    set_label(g_pet_emotion, emotion);
}

void update_affection(const std::string& affection) {
    set_label(g_pet_affection, "亲密度 " + affection);
}

void update_relationship(const std::string& relationship) {
    set_label(g_pet_relationship, relationship);
}

void update_strategy(const std::string& strategy) {
    LV_UNUSED(strategy);
}

void apply_brain_state(const brain::BrainState& state) {
    update_persona_name(state.personaName);
    update_emotion(state.emotion);
    update_affection(std::to_string(state.affinity));
    update_relationship(state.relationship);
}

}  // namespace pet_ui::overlay::status_bar
