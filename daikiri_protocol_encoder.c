#include "daikiri_protocol_encoder.h"

#include "daikiri_protocol.h"

#define GET_BIT(x, n) (((x) >> (n)) & 1)

void daikiri_protocol_encode_fan_mode(DaikiriProtocol* package) {
    uint8_t code;
    if(package->fan_mode == DAIKIRI_FAN_MODE_AUTO) {
        code = 0x1;
    }
    if(package->fan_mode == DAIKIRI_FAN_MODE_MIN) {
        code = 0x8;
    }
    if(package->fan_mode == DAIKIRI_FAN_MODE_MEDIUM) {
        code = 0x4;
    }
    if(package->fan_mode == DAIKIRI_FAN_MODE_MAX) {
        code = 0x2;
    }
    if(package->fan_mode == DAIKIRI_FAN_MODE_QUIET) {
        code = 0x9;
    }
    if(package->fan_mode == DAIKIRI_FAN_MODE_TURBO) {
        code = 0x3;
    }
    package->raw = package->raw | ((uint64_t)((uint64_t)(code) << 12));
}

void daikiri_protocol_encode_is_toggle_power(DaikiriProtocol* protocol) {
    protocol->raw = protocol->raw | ((uint64_t)((uint64_t)(protocol->is_toggle_power) << 59));
}

void daikiri_protocol_encode_mode(DaikiriProtocol* protocol) {
    uint8_t code;
    if(protocol->mode == DAIKIRI_MODE_COOL) {
        code = 0x2;
    }
    if(protocol->mode == DAIKIRI_MODE_DRY) {
        code = 0x1;
    }
    if(protocol->mode == DAIKIRI_MODE_FAN) {
        code = 0x4;
    }
    if(protocol->mode == DAIKIRI_MODE_HEAT) {
        code = 0x8;
    }
    if(protocol->mode == DAIKIRI_MODE_AUTO) {
        code = 0xa;
    }
    protocol->raw = protocol->raw | ((uint64_t)((uint64_t)(code) << 8));
}

void daikiri_protocol_encode_current_time_hours(DaikiriProtocol* protocol) {
    protocol->raw = protocol->raw | ((uint64_t)((uint64_t)(protocol->current_time_hours) << 24));
}

void daikiri_protocol_encode_current_time_minutes(DaikiriProtocol* protocol) {
    protocol->raw = protocol->raw | ((uint64_t)((uint64_t)(protocol->current_time_minutes) << 16));
}

void daikiri_protocol_encode_temperature(DaikiriProtocol* protocol) {
    protocol->raw = protocol->raw | ((uint64_t)((uint64_t)(protocol->temperature) << 48));
}

void daikiri_protocol_encode_is_sleep_mode(DaikiriProtocol* protocol) {
    if(protocol->is_sleep_mode) {
        protocol->raw = protocol->raw | ((uint64_t)((uint64_t)0x1) << 57);
    }
}

void daikiri_protocol_encode_is_swing(DaikiriProtocol* protocol) {
    if(protocol->is_swing) {
        protocol->raw = protocol->raw | ((uint64_t)((uint64_t)0x1) << 56);
    }
}

void daikiri_protocol_encode_is_timer_on_enabled(DaikiriProtocol* protocol) {
    protocol->raw = protocol->raw | ((uint64_t)((uint64_t)(protocol->is_timer_on_enabled) << 39));
}

void daikiri_protocol_encode_is_timer_off_enabled(DaikiriProtocol* protocol) {
    protocol->raw = protocol->raw | ((uint64_t)((uint64_t)(protocol->is_timer_off_enabled) << 47));
}

void daikiri_protocol_encode_hash(DaikiriProtocol* protocol) {
    uint8_t hash = 0;
    for(int i = 0; i < 15; i++) {
        uint64_t mask = 0xFLL << (i * 4);
        hash += ((protocol->raw & mask) >> (i * 4));
    }
    protocol->hash = hash;
    protocol->raw = protocol->raw | ((uint64_t)((uint64_t)(protocol->hash) << 60));
}

void daikiri_protocol_encode_timer_on_hours(DaikiriProtocol* protocol) {
    protocol->raw = protocol->raw | ((uint64_t)((uint64_t)(protocol->timer_on_hours) << 32));
}

void daikiri_protocol_encode_timer_on_minutes(DaikiriProtocol* protocol) {
    uint8_t code;
    if(protocol->timer_on_minutes > 0) {
        code = 0x1;
    } else {
        code = 0x0;
    }
    protocol->raw = protocol->raw | ((uint64_t)((uint64_t)(code) << 38));
}

void daikiri_protocol_encode_timer_off_hours(DaikiriProtocol* protocol) {
    protocol->raw = protocol->raw | ((uint64_t)((uint64_t)(protocol->timer_off_hours) << 40));
}

void daikiri_protocol_encode_timer_off_minutes(DaikiriProtocol* protocol) {
    uint8_t code;
    if(protocol->timer_off_minutes > 0) {
        code = 0x1;
    } else {
        code = 0x0;
    }
    protocol->raw = protocol->raw | ((uint64_t)((uint64_t)(code) << 46));
}

void daikiri_protocol_encode(DaikiriProtocol* protocol) {
    protocol->raw = 0;
    daikiri_protocol_encode_current_time_hours(protocol);
    daikiri_protocol_encode_current_time_minutes(protocol);
    daikiri_protocol_encode_temperature(protocol);
    daikiri_protocol_encode_mode(protocol);
    daikiri_protocol_encode_fan_mode(protocol);
    daikiri_protocol_encode_is_sleep_mode(protocol);
    daikiri_protocol_encode_is_swing(protocol);
    daikiri_protocol_encode_is_toggle_power(protocol);
    daikiri_protocol_encode_is_timer_on_enabled(protocol);
    daikiri_protocol_encode_timer_on_hours(protocol);
    daikiri_protocol_encode_timer_on_minutes(protocol);
    daikiri_protocol_encode_is_timer_off_enabled(protocol);
    daikiri_protocol_encode_timer_off_hours(protocol);
    daikiri_protocol_encode_timer_off_minutes(protocol);

    // MAGIC NUMBERS
    protocol->raw = protocol->raw | 0x16LL;
    protocol->raw = protocol->raw | (0x1LL << 58);

    daikiri_protocol_encode_hash(protocol);
}

// Общий размер - 137 сигналов.
// Формат сигнала:
//   0: PROLOGUE_MARK_1,
//   1: PROLOGUE_MARK_2,
//   2: PROLOGUE_MARK_1,
//   3: PROLOGUE_MARK_2,
//   4: BEGIN_FINISH_MARK,
//   5: PROLOGUE_MARK_4,
//   ...
//   129 сигналов значащих сигналов
//   ...
//   135: EPILOGUE_MARK_1,
//   136: BEGIN_FINISH_MARK.

void daikiri_protocol_constuct_timings(DaikiriProtocol* protocol, uint32_t** timings_out, size_t* timings_cnt_out) {
    size_t timings_cnt = 137;
    uint32_t* timings = malloc(timings_cnt * sizeof(uint32_t));

    timings[0] = PROLOGUE_MARK_1;
    timings[1] = PROLOGUE_MARK_2;
    timings[2] = PROLOGUE_MARK_1;
    timings[3] = PROLOGUE_MARK_2;
    timings[4] = BEGIN_FINISH_MARK;
    timings[5] = PROLOGUE_MARK_4;
    timings[6] = ONE_MARK;

    timings[135] = EPILOGUE_MARK_1;
    timings[136] = BEGIN_FINISH_MARK;

    size_t timings_idx = 7;
    for (int i = 63; i >= 0; i--) {
        if (GET_BIT(protocol->raw, i)) {
            timings[timings_idx] = ONE;
            timings[timings_idx + 1] = ONE_MARK;
            timings_idx++;
        } else {
            timings[timings_idx] = ZERO;
            timings[timings_idx + 1] = ZERO_MARK;
            timings_idx++;
        }
    }

    *timings_out = timings;
    *timings_cnt_out = timings_cnt;

    //    frequency: 38000
    //    duty_cycle: 0.330000
}