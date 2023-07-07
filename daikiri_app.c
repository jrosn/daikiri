#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <gui/elements.h>
#include <gui/view.h>
#include <locale/locale.h>
#include <infrared.h>
#include <infrared_worker.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <dialogs/dialogs.h>
#include <inttypes.h>

#include "daikiri_decoder.h"

#define TAG "Daikiri"

/* --- PRINTF_BYTE_TO_BINARY macro's --- */
#define PRINTF_BINARY_PATTERN_INT8 "%c%c%c%c%c%c%c%c"
#define PRINTF_BYTE_TO_BINARY_INT8(i)                                                       \
    (((i)&0x80ll) ? '1' : '0'), (((i)&0x40ll) ? '1' : '0'), (((i)&0x20ll) ? '1' : '0'),     \
        (((i)&0x10ll) ? '1' : '0'), (((i)&0x08ll) ? '1' : '0'), (((i)&0x04ll) ? '1' : '0'), \
        (((i)&0x02ll) ? '1' : '0'), (((i)&0x01ll) ? '1' : '0')

#define PRINTF_BINARY_PATTERN_INT16 PRINTF_BINARY_PATTERN_INT8 PRINTF_BINARY_PATTERN_INT8
#define PRINTF_BYTE_TO_BINARY_INT16(i) \
    PRINTF_BYTE_TO_BINARY_INT8((i) >> 8), PRINTF_BYTE_TO_BINARY_INT8(i)
#define PRINTF_BINARY_PATTERN_INT32 PRINTF_BINARY_PATTERN_INT16 PRINTF_BINARY_PATTERN_INT16
#define PRINTF_BYTE_TO_BINARY_INT32(i) \
    PRINTF_BYTE_TO_BINARY_INT16((i) >> 16), PRINTF_BYTE_TO_BINARY_INT16(i)
#define PRINTF_BINARY_PATTERN_INT64 PRINTF_BINARY_PATTERN_INT32 PRINTF_BINARY_PATTERN_INT32
#define PRINTF_BYTE_TO_BINARY_INT64(i) \
    PRINTF_BYTE_TO_BINARY_INT32((i) >> 32), PRINTF_BYTE_TO_BINARY_INT32(i)
/* --- end macros --- */

__unused char* daikiri_mode_to_string(DaikiriMode mode) {
    if(mode == DAIKIRI_MODE_COOL) return "C";
    if(mode == DAIKIRI_MODE_DRY) return "D";
    if(mode == DAIKIRI_MODE_FAN) return "F";
    if(mode == DAIKIRI_MODE_HEAT) return "H";
    if(mode == DAIKIRI_MODE_AUTO) return "A";
    return 0;
}

__unused static char* daikiri_fan_mode_to_string(DaikiriFanMode fan_mode) {
    if(fan_mode == DAIKIRI_FAN_MODE_AUTO) return "A";
    if(fan_mode == DAIKIRI_FAN_MODE_MIN) return "1";
    if(fan_mode == DAIKIRI_FAN_MODE_MEDIUM) return "2";
    if(fan_mode == DAIKIRI_FAN_MODE_MAX) return "3";
    if(fan_mode == DAIKIRI_FAN_MODE_QUIET) return "Q";
    if(fan_mode == DAIKIRI_FAN_MODE_TURBO) return "T";
    return 0;
}

static void daikiri_draw_callback(Canvas* canvas, void* context) {
    UNUSED(context);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 5, 12, "I am Daikiri");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 5, 32, "Waiting a signal...");
}

static void daikiri_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);

    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

static void
    daikiri_signal_received_callback(void* context, InfraredWorkerSignal* received_signal) {
    UNUSED(context);

    const uint32_t* timings;
    size_t timings_cnt;
    infrared_worker_get_raw_signal(received_signal, &timings, &timings_cnt);

    DaikiriDecoded* daikiri_decoded = daikiri_decode(timings, timings_cnt);
    if(daikiri_decoded != NULL) {
        NotificationApp* notifications = furi_record_open(RECORD_NOTIFICATION);
        notification_message(notifications, &sequence_success);
        furi_record_close(RECORD_NOTIFICATION);

        FURI_LOG_I(
            TAG,
            PRINTF_BINARY_PATTERN_INT8,
            PRINTF_BYTE_TO_BINARY_INT8(daikiri_decoded->timer_on_hours));

        FURI_LOG_I(
            TAG,
            "raw: 0x%016llx, time: %02x:%02x, mode: %s, temp: %02x, fan: %s, sm: %hhu, sw: %hhu, tp: %hhu, hash: %01x, on: %hhu (%02x:%02x), off: %01x (%02x:%02x)",
            daikiri_decoded->raw,
            daikiri_decoded->current_time_hours,
            daikiri_decoded->current_time_minutes,
            daikiri_mode_to_string(daikiri_decoded->mode),
            daikiri_decoded->temperature,
            daikiri_fan_mode_to_string(daikiri_decoded->fan_mode),
            daikiri_decoded->is_sleep_mode,
            daikiri_decoded->is_swing,
            daikiri_decoded->is_toggle_power,
            daikiri_decoded->hash,
            daikiri_decoded->is_timer_on_enabled,
            daikiri_decoded->timer_on_hours,
            daikiri_decoded->timer_on_minutes,
            daikiri_decoded->is_timer_off_enabled,
            daikiri_decoded->timer_off_hours,
            daikiri_decoded->timer_off_minutes);

        // FURI_LOG_I(
        //     TAG, PRINTF_BINARY_PATTERN_INT64, PRINTF_BYTE_TO_BINARY_INT64(daikiri_decoded->raw));
        char buf[128];
        snprintf(
            buf,
            sizeof(buf),
            "tp: %hhu, hash: %01x\nm: %s, t: %02x, f: %s, sm: %hhu, sw: %hhu\ntime: %02x:%02x\non: %hhu (%02x:%02x), off: %hhu (%02x:%02x)",
            daikiri_decoded->is_toggle_power,
            daikiri_decoded->hash,
            daikiri_mode_to_string(daikiri_decoded->mode),
            daikiri_decoded->temperature,
            daikiri_fan_mode_to_string(daikiri_decoded->fan_mode),
            daikiri_decoded->is_sleep_mode,
            daikiri_decoded->is_swing,
            daikiri_decoded->current_time_hours,
            daikiri_decoded->current_time_minutes,
            daikiri_decoded->is_timer_on_enabled,
            daikiri_decoded->timer_on_hours,
            daikiri_decoded->timer_on_minutes,
            daikiri_decoded->is_timer_off_enabled,
            daikiri_decoded->timer_off_hours,
            daikiri_decoded->timer_off_minutes);

        DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
        DialogMessage* message = dialog_message_alloc();
        dialog_message_set_header(message, "Received signal!", 3, 2, AlignLeft, AlignTop);
        dialog_message_set_text(message, buf, 3, 15, AlignLeft, AlignTop);
        dialog_message_show(dialogs, message);
        dialog_message_free(message);
        furi_record_close(RECORD_DIALOGS);
    }

    daikiri_decoded_free(daikiri_decoded);
}

int32_t daikiri_app(void* p) {
    UNUSED(p);

    InfraredWorker* worker = infrared_worker_alloc();
    infrared_worker_rx_enable_signal_decoding(worker, false);
    infrared_worker_rx_start(worker);
    infrared_worker_rx_set_received_signal_callback(
        worker, daikiri_signal_received_callback, NULL);
    infrared_worker_rx_enable_blink_on_receiving(worker, true);

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, daikiri_draw_callback, view_port);
    view_port_input_callback_set(view_port, daikiri_input_callback, event_queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    InputEvent event;
    bool running = true;
    while(running) {
        if(furi_message_queue_get(event_queue, &event, 100) == FuriStatusOk) {
            if((event.type == InputTypePress) || (event.type == InputTypeRepeat)) {
                switch(event.key) {
                case InputKeyBack:
                    running = false;
                    break;
                default:
                    break;
                }
            }
        }
    }

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);

    infrared_worker_rx_stop(worker);
    infrared_worker_free(worker);
    return 0;
}
