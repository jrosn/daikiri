#include "../daikiri_app_i.h"
#include "../daikiri_protocol.h"
#include "../daikiri_protocol_encoder.h"
#include "../daikiri_custom_event.h"

typedef enum {
    BUTTON_TOGGLE_POWER,
    BUTTON_MODE,
    BUTTON_TEMP_UP,
    BUTTON_TEMP_DOWN,
    BUTTON_FAN_MODE,
    BUTTON_UNUSED,
    LABEL_TEMPERATURE
} ButtonId;

const Icon* mode_icons[5][2] = {
    [DAIKIRI_MODE_HEAT] = {&I_heat_19x20, &I_heat_hover_19x20},
    [DAIKIRI_MODE_COOL] = {&I_cold_19x20, &I_cold_hover_19x20},
    [DAIKIRI_MODE_DRY] = {&I_dry_19x20, &I_dry_hover_19x20},
    [DAIKIRI_MODE_FAN] = {&I_fan_speed_4_19x20, &I_fan_speed_4_hover_19x20},
    [DAIKIRI_MODE_AUTO] = {&I_auto_19x20, &I_auto_hover_19x20},
};

const Icon* fan_mode_icons[6][2] = {
    [DAIKIRI_FAN_MODE_AUTO] = {&I_auto_19x20, &I_auto_hover_19x20},
    [DAIKIRI_FAN_MODE_MIN] = {&I_fan_speed_1_19x20, &I_fan_speed_1_hover_19x20},
    [DAIKIRI_FAN_MODE_MEDIUM] = {&I_fan_speed_2_19x20, &I_fan_speed_2_hover_19x20},
    [DAIKIRI_FAN_MODE_MAX] = {&I_fan_speed_3_19x20, &I_fan_speed_3_hover_19x20},
    [DAIKIRI_FAN_MODE_TURBO] = {&I_fan_speed_4_19x20, &I_fan_speed_4_hover_19x20},
    [DAIKIRI_FAN_MODE_QUIET] = {&I_fan_silent_19x20, &I_fan_silent_hover_19x20}};

void daikiri_scene_universal_common_item_callback(void* context, uint32_t index) {
    DaikiriApp* app = context;
    uint32_t event = daikiri_custom_event_pack(DaikiriCustomEventTypeButtonSelected, index);
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

FuriString* temperature_buffer;

void daikiri_scene_main_on_enter(void* context) {
    DaikiriApp* app = context;
    view_stack_add_view(app->view_stack, ac_remote_panel_get_view(app->ac_remote_panel));

    app->state = daikiri_protocol_alloc_default();
    temperature_buffer = furi_string_alloc();

    ac_remote_panel_reserve(app->ac_remote_panel, 2, 3);
    ac_remote_panel_add_label(app->ac_remote_panel, 0, 6, 11, FontPrimary, "Daikiri");

    ac_remote_panel_add_item(
        app->ac_remote_panel,
        BUTTON_TOGGLE_POWER,
        0,
        0,
        6,
        17,
        &I_off_19x20,
        &I_off_hover_19x20,
        daikiri_scene_universal_common_item_callback,
        context);
    ac_remote_panel_add_icon(app->ac_remote_panel, 9, 39, &I_off_text_14x5);

    ac_remote_panel_add_item(
        app->ac_remote_panel,
        BUTTON_MODE,
        1,
        0,
        39,
        17,
        mode_icons[app->state->mode][0],
        mode_icons[app->state->mode][1],
        daikiri_scene_universal_common_item_callback,
        context);
    ac_remote_panel_add_icon(app->ac_remote_panel, 39, 39, &I_mode_text_20x5);

    ac_remote_panel_add_item(
        app->ac_remote_panel,
        BUTTON_FAN_MODE,
        1,
        1,
        39,
        54,
        fan_mode_icons[app->state->fan_mode][0],
        fan_mode_icons[app->state->fan_mode][1],
        daikiri_scene_universal_common_item_callback,
        context);
    ac_remote_panel_add_icon(app->ac_remote_panel, 41, 76, &I_fan_text_14x5);

    ac_remote_panel_add_icon(app->ac_remote_panel, 0, 63, &I_frame_30x39);
    ac_remote_panel_add_item(
        app->ac_remote_panel,
        BUTTON_TEMP_UP,
        0,
        1,
        3,
        51,
        &I_tempup_24x21,
        &I_tempup_hover_24x21,
        daikiri_scene_universal_common_item_callback,
        context);
    ac_remote_panel_add_item(
        app->ac_remote_panel,
        BUTTON_TEMP_DOWN,
        0,
        2,
        3,
        93,
        &I_tempdown_24x21,
        &I_tempdown_hover_24x21,
        daikiri_scene_universal_common_item_callback,
        context);
    ac_remote_panel_add_item(
            app->ac_remote_panel,
            BUTTON_UNUSED,
            1,
            2,
            37,
            93,
            &I_tempdown_24x21,
            &I_tempdown_hover_24x21,
            daikiri_scene_universal_common_item_callback,
            context);
    FuriString* temp_str = furi_string_alloc_printf("%hhu", app->state->temperature);
    ac_remote_panel_add_label(
        app->ac_remote_panel,
        LABEL_TEMPERATURE,
        4,
        86,
        FontKeyboard,
        furi_string_get_cstr(temp_str));

    view_set_orientation(view_stack_get_view(app->view_stack), ViewOrientationVertical);
    view_dispatcher_switch_to_view(app->view_dispatcher, DaikiriAppViewStack);
}

void daikiri_scene_main_update_current_time_in_state(DaikiriApp *app) {
    uint32_t timestamp = furi_hal_rtc_get_timestamp();
    DateTime currentDateTime = {};
    datetime_timestamp_to_datetime(timestamp, &currentDateTime);
    app->state->current_time_hours = currentDateTime.hour;
    app->state->current_time_hours = currentDateTime.minute;
}

bool daikiri_scene_main_on_event(void* context, SceneManagerEvent event) {
    DaikiriApp* app = context;
    UNUSED(app);
    UNUSED(event);
    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom) {
        uint16_t event_type;
        int16_t event_value;
        daikiri_custom_event_unpack(event.event, &event_type, &event_value);
        if(event_type == DaikiriCustomEventTypeSendCommand) {
            NotificationApp* notifications = furi_record_open(RECORD_NOTIFICATION);
            notification_message(notifications, &sequence_blink_white_100);
            daikiri_scene_main_update_current_time_in_state(app);
            daikiri_protocol_send(app->state);
            app->state->is_toggle_power = false;
            notification_message(notifications, &sequence_blink_stop);
        } else if(event_type == DaikiriCustomEventTypeButtonSelected) {
            switch(event_value) {
            case BUTTON_TOGGLE_POWER:
                app->state->is_toggle_power = true;
                break;
            case BUTTON_FAN_MODE:
                app->state->fan_mode = (app->state->fan_mode + 1) % DAIKIRI_FAN_MODE_CNT;
                ac_remote_panel_item_set_icons(
                    app->ac_remote_panel,
                    BUTTON_FAN_MODE,
                    fan_mode_icons[app->state->fan_mode][0],
                    fan_mode_icons[app->state->fan_mode][0]
                );
                break;
            case BUTTON_MODE:
                app->state->mode = (app->state->mode + 1) % DAIKIRI_MODE_CNT;
                ac_remote_panel_item_set_icons(
                    app->ac_remote_panel,
                    BUTTON_MODE,
                    mode_icons[app->state->mode][0],
                    mode_icons[app->state->mode][0]
                );
                break;
            case BUTTON_TEMP_UP:
                if (app->state->temperature < 30) {
                    app->state->temperature++;
                }
                furi_string_reset(temperature_buffer);
                furi_string_cat_printf(temperature_buffer, "%hhu", app->state->temperature);
                ac_remote_panel_label_set_string(
                    app->ac_remote_panel,
                    LABEL_TEMPERATURE,
                    furi_string_get_cstr(temperature_buffer)
                );
                break;
            case BUTTON_TEMP_DOWN:
                if (app->state->temperature > 16) {
                    app->state->temperature--;
                }
                furi_string_reset(temperature_buffer);
                furi_string_cat_printf(temperature_buffer, "%hhu", app->state->temperature);
                ac_remote_panel_label_set_string(
                    app->ac_remote_panel,
                    LABEL_TEMPERATURE,
                    furi_string_get_cstr(temperature_buffer)
                );
                break;
            default:
                furi_assert(false, "Invalid button_id");
                break;
            }
            view_dispatcher_send_custom_event(
                app->view_dispatcher,
                daikiri_custom_event_pack(DaikiriCustomEventTypeSendCommand, 0)
            );
        }
        consumed = true;
    }
    return consumed;
}

void daikiri_scene_main_on_exit(void* context)
{
//    AC_RemoteApp* ac_remote = context;
//    ACRemotePanel* ac_remote_panel = ac_remote->ac_remote_panel;
//    ac_remote_store_settings(&ac_remote->app_state);
//    hvac_mitsubishi_deinit(ac_remote->hvac_mitsubishi_data);
//    view_stack_remove_view(ac_remote->view_stack, ac_remote_panel_get_view(ac_remote_panel));
//    ac_remote_panel_reset(ac_remote_panel);
    DaikiriApp* app = context;
    ac_remote_panel_free(app->ac_remote_panel);
    furi_string_free(temperature_buffer);
}