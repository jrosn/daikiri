#pragma once

#include <furi.h>

#include "daikiri_protocol.h"
#include "daikiri_defs.h"

void daikiri_protocol_decode_fan_mode(DaikiriProtocol* protocol);

void daikiri_protocol_decode_current_time_hours(DaikiriProtocol* protocol);

void daikiri_protocol_decode_current_time_minutes(DaikiriProtocol* protocol);

void daikiri_protocol_decode_temperature(DaikiriProtocol* protocol);

void daikiri_protocol_decode_is_sleep_mode(DaikiriProtocol* protocol);

void daikiri_protocol_decode_is_swing(DaikiriProtocol* protocol);

void daikiri_protocol_decode_is_toggle_power(DaikiriProtocol* protocol);

void daikiri_protocol_decode_mode(DaikiriProtocol* protocol);

void daikiri_protocol_decode_is_timer_on_enabled(DaikiriProtocol* protocol);

void daikiri_protocol_decode_is_timer_off_enabled(DaikiriProtocol* protocol);

void daikiri_protocol_decode_hash(DaikiriProtocol* protocol);

void daikiri_protocol_decode_timer_on_hours(DaikiriProtocol* protocol);

void daikiri_protocol_decode_timer_on_minutes(DaikiriProtocol* protocol);

void daikiri_protocol_decode_timer_off_hours(DaikiriProtocol* protocol);

void daikiri_protocol_decode_timer_off_minutes(DaikiriProtocol* protocol);

void daikiri_protocol_decode(DaikiriProtocol* protocol);

DaikiriProtocol* daikiri_protocol_interpret_timings(const uint32_t* timings, size_t timings_cnt);