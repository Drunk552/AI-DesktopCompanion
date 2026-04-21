#include "ui/screens/interaction/ui_interaction.h"
#include "app/settings_events.h"
#include "ui/common/ui_style.h"
#include "ui/common/ui_widgets.h"
#include "ui/managers/ui_manager.h"
#include "ui/screens/interaction/strategy_controls_panel.h"
#include "ui/screens/interaction/strategy_file_panel.h"
#include "ui/screens/interaction/strategy_overview_panel.h"
#include "ui/screens/menu/ui_scr_menu.h"
#include <mutex>

namespace {

lv_obj_t* g_screen = nullptr;
lv_obj_t* g_back = nullptr;
pet_ui::screen::interaction::StrategyOverviewPanel g_overview;
pet_ui::screen::interaction::StrategyControlsPanel g_controls;
pet_ui::screen::interaction::StrategyFilePanel g_file;
bool g_bound = false;
std::mutex g_pending_mutex;
app::settings::events::SettingsStateSnapshot g_pending_snapshot;
bool g_pending_snapshot_updated = false;

static void screen_event_cb(lv_event_t* e) {
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    lv_obj_t* target = static_cast<lv_obj_t*>(lv_event_get_target(e));
    if (target == g_back) {
        pet_ui::screen::menu::load_screen();
        return;
    }

    auto* eventBus = pet_ui::manager::UiManager::getInstance()->getEventBus();
    if (target == g_controls.quiet) {
        eventBus->emitTyped(app::settings::events::kDisturbanceModeSet, app::settings::events::DisturbanceModeSetCommand{"quiet"});
    } else if (target == g_controls.normal) {
        eventBus->emitTyped(app::settings::events::kDisturbanceModeSet, app::settings::events::DisturbanceModeSetCommand{"normal"});
    } else if (target == g_controls.clingy) {
        eventBus->emitTyped(app::settings::events::kDisturbanceModeSet, app::settings::events::DisturbanceModeSetCommand{"clingy"});
    } else if (target == g_controls.reset) {
        eventBus->emitTyped(app::settings::events::kStrategyReset, app::settings::events::SimpleActionCommand{});
    } else if (target == g_controls.toggleCheckin) {
        eventBus->emitTyped(app::settings::events::kBehaviorCheckInToggle, app::settings::events::ToggleBehaviorCommand{"checkin"});
    } else if (target == g_controls.toggleTease) {
        eventBus->emitTyped(app::settings::events::kBehaviorTeaseToggle, app::settings::events::ToggleBehaviorCommand{"tease"});
    } else if (target == g_controls.toggleReminder) {
        eventBus->emitTyped(app::settings::events::kBehaviorReminderToggle, app::settings::events::ToggleBehaviorCommand{"reminder"});
    } else if (target == g_controls.toggleSilentNight) {
        eventBus->emitTyped(app::settings::events::kBehaviorSilentNightToggle, app::settings::events::ToggleBehaviorCommand{"silent_night"});
    } else if (target == g_file.exportBtn) {
        eventBus->emitTyped(app::settings::events::kStrategyExport, app::settings::events::SimpleActionCommand{});
    } else if (target == g_file.importFullBtn) {
        eventBus->emitTyped(app::settings::events::kStrategyImport, app::settings::events::StrategyImportCommand{"full"});
    } else if (target == g_file.importTogglesBtn) {
        eventBus->emitTyped(app::settings::events::kStrategyImport, app::settings::events::StrategyImportCommand{"toggles"});
    } else if (target == g_file.importStatsBtn) {
        eventBus->emitTyped(app::settings::events::kStrategyImport, app::settings::events::StrategyImportCommand{"stats"});
    } else if (target == g_file.restoreDefaultsBtn) {
        eventBus->emitTyped(app::settings::events::kStrategyRestoreDefaults, app::settings::events::SimpleActionCommand{});
    }
}

static void screen_cleanup_cb(lv_event_t*) {
    g_screen = nullptr;
    g_back = nullptr;
    g_overview = {};
    g_controls = {};
    g_file = {};
}

static void bind_clicks() {
    lv_obj_add_event_cb(g_controls.quiet, screen_event_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(g_controls.normal, screen_event_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(g_controls.clingy, screen_event_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(g_controls.reset, screen_event_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(g_controls.toggleCheckin, screen_event_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(g_controls.toggleTease, screen_event_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(g_controls.toggleReminder, screen_event_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(g_controls.toggleSilentNight, screen_event_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(g_file.exportBtn, screen_event_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(g_file.importFullBtn, screen_event_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(g_file.importTogglesBtn, screen_event_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(g_file.importStatsBtn, screen_event_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(g_file.restoreDefaultsBtn, screen_event_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(g_back, screen_event_cb, LV_EVENT_CLICKED, nullptr);
}

static void create_screen() {
    auto* manager = pet_ui::manager::UiManager::getInstance();
    g_screen = lv_obj_create(manager->getContentHost());
    pet_ui::style::style_screen_root(g_screen);
    manager->registerScreen(pet_ui::manager::ScreenType::Interaction, &g_screen);
    lv_obj_add_event_cb(g_screen, screen_cleanup_cb, LV_EVENT_DELETE, nullptr);

    g_overview = pet_ui::screen::interaction::create_strategy_overview_panel(g_screen, manager->getTitleFont(), manager->getBodyFont());
    g_controls = pet_ui::screen::interaction::create_strategy_controls_panel(g_screen, manager->getTitleFont(), manager->getBodyFont());
    g_file = pet_ui::screen::interaction::create_strategy_file_panel(g_screen, manager->getTitleFont(), manager->getBodyFont());

    g_back = pet_ui::widgets::create_back_button(g_screen, "返回菜单", manager->getBodyFont());
    bind_clicks();
}

}  // namespace

namespace pet_ui::screen::interaction {

void bind_event_bus(AppEventBus& eventBus) {
    if (g_bound) {
        return;
    }
    g_bound = true;

    eventBus.subscribeTyped<::app::settings::events::SettingsStateSnapshot>(::app::settings::events::kStateSnapshotChanged, [](const ::app::settings::events::SettingsStateSnapshot& snapshot) {
        std::lock_guard<std::mutex> lock(g_pending_mutex);
        g_pending_snapshot = snapshot;
        g_pending_snapshot_updated = true;
    });
}

void process_pending() {
    app::settings::events::SettingsStateSnapshot snapshot;
    bool updated = false;
    {
        std::lock_guard<std::mutex> lock(g_pending_mutex);
        snapshot = g_pending_snapshot;
        updated = g_pending_snapshot_updated;
        g_pending_snapshot_updated = false;
    }
    if (updated) {
        apply_settings_snapshot(snapshot);
    }
}

void load_screen() {
    auto* manager = pet_ui::manager::UiManager::getInstance();
    if (!g_screen) {
        create_screen();
    }
    manager->resetKeypadGroup();
    manager->addObjToGroup(g_controls.quiet);
    manager->addObjToGroup(g_controls.normal);
    manager->addObjToGroup(g_controls.clingy);
    manager->addObjToGroup(g_controls.reset);
    manager->addObjToGroup(g_controls.toggleCheckin);
    manager->addObjToGroup(g_controls.toggleTease);
    manager->addObjToGroup(g_controls.toggleReminder);
    manager->addObjToGroup(g_controls.toggleSilentNight);
    manager->addObjToGroup(g_file.exportBtn);
    manager->addObjToGroup(g_file.importFullBtn);
    manager->addObjToGroup(g_file.importTogglesBtn);
    manager->addObjToGroup(g_file.importStatsBtn);
    manager->addObjToGroup(g_file.restoreDefaultsBtn);
    manager->addObjToGroup(g_back);
    lv_group_focus_obj(g_controls.normal ? g_controls.normal : g_back);
    manager->loadContentScreen(pet_ui::manager::ScreenType::Interaction, g_screen);
}

void set_disturbance_mode(const std::string& mode) { set_disturbance_mode(g_controls, mode); }
void set_persona_style(const std::string& personaStyle) { set_persona_style(g_overview, personaStyle); }
void set_strategy_summary(const std::string& strategySummary) { set_strategy_summary(g_overview, strategySummary); }
void set_last_behavior(const std::string& behavior) { set_last_behavior(g_overview, behavior); }
void set_last_interaction(const std::string& interaction) { set_last_interaction(g_overview, interaction); }
void set_quiet_hour(const std::string& quietHour) { set_quiet_hour(g_overview, quietHour); }
void set_behavior_stats(const std::string& stats) { set_behavior_stats(g_overview, stats); }
void set_strategy_file_path(const std::string& path) { set_strategy_file_path(g_overview, path); }
void set_import_result(const std::string& result) { set_import_result(g_overview, result); }
void set_import_error(const std::string& error) { set_import_error(g_overview, error); }
void set_behavior_toggle(const std::string& key, const std::string& enabled) { set_behavior_toggle(g_controls, key, enabled); }
void apply_settings_snapshot(const ::app::settings::events::SettingsStateSnapshot& snapshot) {
    set_disturbance_mode(g_controls, snapshot.disturbanceMode);
    set_persona_style(g_overview, snapshot.personaStyle);
    set_strategy_summary(g_overview, snapshot.strategySummary);
    set_last_behavior(g_overview, snapshot.lastBehavior);
    set_last_interaction(g_overview, snapshot.lastInteraction);
    set_quiet_hour(g_overview, snapshot.quietHour);
    set_behavior_stats(g_overview, snapshot.behaviorStats);
    set_strategy_file_path(g_overview, snapshot.strategyFilePath);
    set_import_result(g_overview, snapshot.importResult);
    set_import_error(g_overview, snapshot.importError);
    set_behavior_toggle(g_controls, "checkin", snapshot.checkInEnabled ? "1" : "0");
    set_behavior_toggle(g_controls, "tease", snapshot.teaseEnabled ? "1" : "0");
    set_behavior_toggle(g_controls, "reminder", snapshot.reminderEnabled ? "1" : "0");
    set_behavior_toggle(g_controls, "silent_night", snapshot.silentNightEnabled ? "1" : "0");
}

}  // namespace pet_ui::screen::interaction
