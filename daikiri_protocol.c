#include "daikiri_protocol.h"

#include <furi.h>

void daikiri_protocol_free(DaikiriProtocol *ptr) {
    free(ptr);
}

DaikiriProtocol* daikiri_protocol_alloc_default() {
    DaikiriProtocol *protocol = malloc(sizeof(DaikiriProtocol));
    protocol->fan_mode = DAIKIRI_FAN_MODE_AUTO;
    protocol->mode = DAIKIRI_MODE_AUTO;
    protocol->current_time_hours = 12;
    protocol->current_time_minutes = 00;
    protocol->temperature = 23;
    protocol->is_sleep_mode = false;
    protocol->is_swing = false;
    protocol->is_toggle_power = false;

    protocol->is_timer_on_enabled = false;
    protocol->timer_on_hours = 0;
    protocol->timer_on_minutes = 0;

    protocol->is_timer_off_enabled = false;
    protocol->timer_off_hours = 0;
    protocol->timer_off_minutes = 0;

    return protocol;
}

char* daikiri_protocol_mode_to_char(DaikiriMode mode) {
    if (mode == DAIKIRI_MODE_COOL) return "C";
    if (mode == DAIKIRI_MODE_DRY) return "D";
    if (mode == DAIKIRI_MODE_FAN) return "F";
    if (mode == DAIKIRI_MODE_HEAT) return "H";
    if (mode == DAIKIRI_MODE_AUTO) return "A";
    return 0;
}

char* daikiri_protocol_fan_mode_to_char(DaikiriFanMode fan_mode) {
    if (fan_mode == DAIKIRI_FAN_MODE_AUTO) return "A";
    if (fan_mode == DAIKIRI_FAN_MODE_MIN) return "1";
    if (fan_mode == DAIKIRI_FAN_MODE_MEDIUM) return "2";
    if (fan_mode == DAIKIRI_FAN_MODE_MAX) return "3";
    if (fan_mode == DAIKIRI_FAN_MODE_QUIET) return "Q";
    if (fan_mode == DAIKIRI_FAN_MODE_TURBO) return "T";
    return 0;
}

void daikiri_protocol_to_string(FuriString* str, DaikiriProtocol* protocol) {
    const char* format =
            "DaikiriProtocol["
            "raw: 0x%016llx, "
            "time: %02hhu:%02hhu, "
            "mode: %s, "
            "temp: %02hhu, "
            "fan: %s, "
            "sm: %hhu, "
            "sw: %hhu, "
            "tp: %hhu, "
            "hash: %01x, "
            "on: %hhu (%02hhu:%02hhu), "
            "off: %hhu (%02hhu:%02hhu)]";
    furi_string_cat_printf(
            str,
            format,
            protocol->raw,
            protocol->current_time_hours,
            protocol->current_time_minutes,
            daikiri_protocol_mode_to_char(protocol->mode),
            protocol->temperature,
            daikiri_protocol_fan_mode_to_char(protocol->fan_mode),
            protocol->is_sleep_mode,
            protocol->is_swing,
            protocol->is_toggle_power,
            protocol->hash,
            protocol->is_timer_on_enabled,
            protocol->timer_on_hours,
            protocol->timer_on_minutes,
            protocol->is_timer_off_enabled,
            protocol->timer_off_hours,
            protocol->timer_off_minutes);
}