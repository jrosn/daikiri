#include <furi.h>

#include <gui/gui.h>
#include <gui/elements.h>
#include <gui/view.h>
#include <infrared.h>
#include <infrared_worker.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <dialogs/dialogs.h>

#include "daikiri_protocol.h"
#include "daikiri_protocol_decoder.h"
#include "daikiri_protocol_encoder.h"
#include "daikiri_defs.h"

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

static void daikiri_signal_received_callback(void* context, InfraredWorkerSignal* received_signal) {
    UNUSED(context);

    const uint32_t* timings;
    size_t timings_cnt;
    infrared_worker_get_raw_signal(received_signal, &timings, &timings_cnt);

    DaikiriProtocol* decoded = daikiri_protocol_interpret_timings(timings, timings_cnt);

    if(decoded != NULL) {
        daikiri_protocol_decode(decoded);

        DaikiriProtocol* protocol = malloc(sizeof(DaikiriProtocol));
        memcpy(protocol, decoded, sizeof(DaikiriProtocol));
        protocol->raw = 0;
        daikiri_protocol_encode(protocol);

        FURI_LOG_I(TAG, "======================================================================");
        FURI_LOG_I(TAG, PRINTF_BINARY_PATTERN_INT64, PRINTF_BYTE_TO_BINARY_INT64(decoded->raw));
        FURI_LOG_I(TAG, PRINTF_BINARY_PATTERN_INT64, PRINTF_BYTE_TO_BINARY_INT64(protocol->raw));

        FuriString* decoded_string = furi_string_alloc();
        for (size_t i = 0; i < timings_cnt; i++) {
            furi_string_cat_printf(decoded_string, "\r\n%ld", timings[i]);
        }
        FURI_LOG_I(TAG, "%s", furi_string_get_cstr(decoded_string));


        uint32_t* timings1 = NULL;
        size_t timings_cnt1 = 0;
        daikiri_protocol_encode(protocol);
        daikiri_protocol_constuct_timings(protocol, &timings1, &timings_cnt1);
        FuriString* decoded_string1 = furi_string_alloc();
        for (size_t i = 0; i < timings_cnt1; i++) {
            furi_string_cat_printf(decoded_string1, "\r\n%ld", timings1[i]);
        }
        FURI_LOG_I("DaikiriEncoder", "%s", furi_string_get_cstr(decoded_string1));

        NotificationApp* notifications = furi_record_open(RECORD_NOTIFICATION);
        notification_message(notifications, &sequence_success);
        furi_record_close(RECORD_NOTIFICATION);

        FuriString* string = furi_string_alloc();
        daikiri_protocol_to_string(string, decoded);
        FURI_LOG_I(
            TAG,
            "Received signal: %s",
            furi_string_get_cstr(string)
        );

        furi_string_free(string);
        char buf[128];
        snprintf(
            buf,
            sizeof(buf),
            "tp: %hhu, hash: %01x\nm: %s, t: %02hhu, f: %s, sm: %hhu, sw: %hhu\ntime: %02hhu:%02hhu\non: %hhu (%02hhu:%02hhu), off: %hhu (%02hhu:%02hhu)",
            decoded->is_toggle_power,
            decoded->hash,
            daikiri_protocol_mode_to_char(decoded->mode),
            decoded->temperature,
            daikiri_protocol_fan_mode_to_char(decoded->fan_mode),
            decoded->is_sleep_mode,
            decoded->is_swing,
            decoded->current_time_hours,
            decoded->current_time_minutes,
            decoded->is_timer_on_enabled,
            decoded->timer_on_hours,
            decoded->timer_on_minutes,
            decoded->is_timer_off_enabled,
            decoded->timer_off_hours,
            decoded->timer_off_minutes
        );
        DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
        DialogMessage* message = dialog_message_alloc();
        dialog_message_set_header(message, "Received signal!", 3, 2, AlignLeft, AlignTop);
        dialog_message_set_text(message, buf, 3, 15, AlignLeft, AlignTop);
        dialog_message_show(dialogs, message);
        dialog_message_free(message);
        furi_record_close(RECORD_DIALOGS);
    }

    daikiri_protocol_free(decoded);
}

int32_t daikiri_decoder_app(void* p) {
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