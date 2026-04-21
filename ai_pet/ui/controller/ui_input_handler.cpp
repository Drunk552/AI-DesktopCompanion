#include "ui/controller/ui_input_handler.h"
#include "brain/brain_events.h"
#include <cstring>

namespace {

struct EmitContext {
    AppEventBus* eventBus;
    std::string eventType;
    std::string data;
};

struct SubmitContext {
    AppEventBus* eventBus;
    lv_obj_t* textarea;
};

void delete_emit_context(lv_event_t* e) {
    delete static_cast<EmitContext*>(lv_event_get_user_data(e));
}

void delete_submit_context(lv_event_t* e) {
    delete static_cast<SubmitContext*>(lv_event_get_user_data(e));
}

void emit_click(lv_event_t* e) {
    EmitContext* ctx = static_cast<EmitContext*>(lv_event_get_user_data(e));
    if (!ctx || !ctx->eventBus) {
        return;
    }
    ctx->eventBus->emit(ctx->eventType, ctx->data);
}

void submit_text(lv_event_t* e) {
    SubmitContext* ctx = static_cast<SubmitContext*>(lv_event_get_user_data(e));
    if (!ctx || !ctx->eventBus || !ctx->textarea) {
        return;
    }

    const char* text = lv_textarea_get_text(ctx->textarea);
    if (!text || std::strlen(text) == 0) {
        return;
    }

    ctx->eventBus->emitTyped(brain::events::kUserInputText, brain::events::UserTextInputEvent{text});
    lv_textarea_set_text(ctx->textarea, "");
}

}  // namespace

namespace ui_input_handler {

void bind_emit_on_click(lv_obj_t* obj, AppEventBus& eventBus, const std::string& eventType, const std::string& data) {
    EmitContext* context = new EmitContext{&eventBus, eventType, data};
    lv_obj_add_event_cb(obj, emit_click, LV_EVENT_CLICKED, context);
    lv_obj_add_event_cb(obj, delete_emit_context, LV_EVENT_DELETE, context);
}

void bind_text_submit(lv_obj_t* trigger, lv_event_code_t eventCode, lv_obj_t* textarea, AppEventBus& eventBus) {
    SubmitContext* context = new SubmitContext{&eventBus, textarea};
    lv_obj_add_event_cb(trigger, submit_text, eventCode, context);
    lv_obj_add_event_cb(trigger, delete_submit_context, LV_EVENT_DELETE, context);
}

}  // namespace ui_input_handler
