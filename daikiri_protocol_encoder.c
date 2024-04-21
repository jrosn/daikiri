#include "daikiri_protocol_encoder.h"

#include "daikiri_protocol.h"
#include "infrared_transmit.h"

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
    uint8_t code = ((protocol->current_time_hours / 10) << 4) + protocol->current_time_hours % 10;
    protocol->raw = protocol->raw | ((uint64_t)((uint64_t)(code) << 24));
}

void daikiri_protocol_encode_current_time_minutes(DaikiriProtocol* protocol) {
    uint8_t code = ((protocol->current_time_minutes / 10) << 4) + protocol->current_time_minutes % 10;
    protocol->raw = protocol->raw | ((uint64_t)((uint64_t)(code) << 16));
}

void daikiri_protocol_encode_temperature(DaikiriProtocol* protocol) {
    uint8_t code = ((protocol->temperature / 10) << 4) + protocol->temperature % 10;
    protocol->raw = protocol->raw | ((uint64_t)((uint64_t)(code) << 48));
}

void daikiri_protocol_encode_is_sleep_mode(DaikiriProtocol* protocol) {
    if(protocol->is_sleep_mode) {
        protocol->raw = protocol->raw | (0x1LL << 57);
    }
}

void daikiri_protocol_encode_is_swing(DaikiriProtocol* protocol) {
    if(protocol->is_swing) {
        protocol->raw = protocol->raw | (0x1LL << 56);
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
    uint8_t code = ((protocol->timer_on_hours / 10) << 4) + protocol->timer_on_hours % 10;
    protocol->raw = protocol->raw | ((uint64_t)((uint64_t)(code) << 32));
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
    uint8_t code = ((protocol->timer_off_hours / 10) << 4) + protocol->timer_off_hours % 10;
    protocol->raw = protocol->raw | ((uint64_t)((uint64_t)(code) << 40));
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
    furi_assert(0 <= protocol->current_time_hours && protocol->current_time_hours <= 59);
    furi_assert(0 <= protocol->current_time_minutes && protocol->current_time_minutes <= 59);
    furi_assert(16 <= protocol->temperature && protocol->temperature <= 30);
    assert(protocol->mode != DAIKIRI_MODE_CNT);
    assert(protocol->mode != DAIKIRI_FAN_MODE_CNT);
    assert(protocol->mode != DAIKIRI_MODE_FAN ||
        protocol->fan_mode == DAIKIRI_FAN_MODE_MAX ||
        protocol->fan_mode == DAIKRI_FAN_MODE_MEDIUM ||
        protocol->fan_mode == DAIKIRI_FAN_MODE_MIN);

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

    timings[135] = 20199;
    timings[136] = BEGIN_FINISH_MARK;

    size_t timings_idx = 7;
    for (int i = 0; i < 64; i++) {
        if (GET_BIT(protocol->raw, i)) {
            timings[timings_idx++] = ONE;
            timings[timings_idx++] = ONE_MARK;
        } else {
            timings[timings_idx++] = ZERO;
            timings[timings_idx++] = ZERO_MARK;
        }
    }

    *timings_out = timings;
    *timings_cnt_out = timings_cnt;
}

void daikiri_protocol_send(DaikiriProtocol* protocol) {
    uint32_t* timings = NULL;
    size_t timings_cnt = 0;
    daikiri_protocol_encode(protocol);
    daikiri_protocol_constuct_timings(protocol, &timings, &timings_cnt);
    FuriString* decoded_string = furi_string_alloc();
    for (size_t i = 0; i < timings_cnt; i++) {
        furi_string_cat_printf(decoded_string, "\r\n%ld", timings[i]);
    }
    FURI_LOG_I("DaikiriEncoder", "%s", furi_string_get_cstr(decoded_string));

    infrared_send_raw_ext(timings, timings_cnt, true, 38000, 0.33f);
    free(timings);
}