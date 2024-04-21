#include "daikiri_protocol.h"

#include <furi.h>

void daikiri_protocol_free(DaikiriProtocol* ptr) {
    free(ptr);
}

DaikiriProtocol* daikiri_protocol_alloc_default() {
    DaikiriProtocol* protocol = malloc(sizeof(DaikiriProtocol));
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