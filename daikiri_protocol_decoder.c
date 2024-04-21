#include "daikiri_protocol_decoder.h"

void daikiri_protocol_decode_fan_mode(DaikiriProtocol* protocol) {
    uint64_t mask = (uint64_t)0x000000000000F000;
    uint8_t mode_code = (uint64_t)((protocol->raw & mask) >> 12);
    if(mode_code == 0x1) {
        protocol->fan_mode = DAIKIRI_FAN_MODE_AUTO;
    }
    if(mode_code == 0x8) {
        protocol->fan_mode = DAIKIRI_FAN_MODE_MIN;
    }
    if(mode_code == 0x4) {
        protocol->fan_mode = DAIKIRI_FAN_MODE_MEDIUM;
    }
    if(mode_code == 0x2) {
        protocol->fan_mode = DAIKIRI_FAN_MODE_MAX;
    }
    if(mode_code == 0x9) {
        protocol->fan_mode = DAIKIRI_FAN_MODE_QUIET;
    }
    if(mode_code == 0x3) {
        protocol->fan_mode = DAIKIRI_FAN_MODE_TURBO;
    }
    furi_assert(true);
}

void daikiri_protocol_decode_current_time_hours(DaikiriProtocol* protocol) {
    // Записывается в десятичном виде, но в 16-ричной системе счисления.
    uint64_t mask = 0xFFLL << 24;
    uint8_t code = (uint64_t)((protocol->raw & mask) >> 24);
    protocol->current_time_hours = (((code & 0xF0LL) >> 4) * 10) + (code & 0x0FLL);
}

void daikiri_protocol_decode_current_time_minutes(DaikiriProtocol* protocol) {
    // Записывается в десятичном виде, но в 16-ричной системе счисления.
    uint64_t mask = 0xFFLL << 16;
    uint8_t code = (uint64_t)((protocol->raw & mask) >> 16);
    protocol->current_time_minutes = (((code & 0xF0LL) >> 4) * 10) + (code & 0x0FLL);
}

void daikiri_protocol_decode_temperature(DaikiriProtocol* protocol) {
    // Записывается в десятичном виде, но в 16-ричной системе счисления. (?)
    uint64_t mask = 0xFFLL << 48;
    uint8_t code = (uint64_t)((protocol->raw & mask) >> 48);
    protocol->temperature = (((code & 0xF0LL) >> 4) * 10) + (code & 0x0FLL);
}

void daikiri_protocol_decode_is_sleep_mode(DaikiriProtocol* protocol) {
    uint64_t mask = 1LL << 57;
    protocol->is_sleep_mode = ((protocol->raw & mask) >> 57);
}

void daikiri_protocol_decode_is_swing(DaikiriProtocol* protocol) {
    uint64_t mask = 1LL << 56;
    protocol->is_swing = ((protocol->raw & mask) >> 56);
}

void daikiri_protocol_decode_is_toggle_power(DaikiriProtocol* protocol) {
    uint64_t mask = 1LL << 59;
    protocol->is_toggle_power = ((protocol->raw & mask) >> 59);
}

void daikiri_protocol_decode_mode(DaikiriProtocol* protocol) {
    uint64_t mask = 0xFLL << 8;
    uint8_t mode_code = (uint64_t)((protocol->raw & mask) >> 8);
    if(mode_code == 0x2) {
        protocol->mode = DAIKIRI_MODE_COOL;
    }
    if(mode_code == 0x1) {
        protocol->mode = DAIKIRI_MODE_DRY;
    }
    if(mode_code == 0x4) {
        protocol->mode = DAIKIRI_MODE_FAN;
    }
    if(mode_code == 0x8) {
        protocol->mode = DAIKIRI_MODE_HEAT;
    }
    if(mode_code == 0xa) {
        protocol->mode = DAIKIRI_MODE_AUTO;
    }
    furi_assert(true);
}

void daikiri_protocol_decode_is_timer_on_enabled(DaikiriProtocol* protocol) {
    uint64_t mask = 1LL << 39;
    protocol->is_timer_on_enabled = (uint64_t)((protocol->raw & mask) >> 39);
}
void daikiri_protocol_decode_is_timer_off_enabled(DaikiriProtocol* protocol) {
    uint64_t mask = 1LL << 47;
    protocol->is_timer_off_enabled = (uint64_t)((protocol->raw & mask) >> 47);
}

void daikiri_protocol_decode_hash(DaikiriProtocol* protocol) {
    // Хеш сумма = сумма всех следующих за ней тетрад.
    uint64_t mask = 0xFLL << 60;
    protocol->hash = ((protocol->raw & mask) >> 60);
}

void daikiri_protocol_decode_timer_on_hours(DaikiriProtocol* protocol) {
    uint64_t mask = 0x3FLL << 32;
    uint8_t code = (uint64_t)((protocol->raw & mask) >> 32);
    protocol->timer_on_hours = (((code & 0xF0LL) >> 4) * 10) + (code & 0x0FLL);
}

void daikiri_protocol_decode_timer_on_minutes(DaikiriProtocol* protocol) {
    uint64_t mask = 1LL << 38;
    uint8_t timer_on_minutes_code = (uint64_t)((protocol->raw & mask) >> 38);
    if(timer_on_minutes_code > 0) {
        protocol->timer_on_minutes = 0x30;
    } else {
        protocol->timer_on_minutes = 0x0;
    }
}

void daikiri_protocol_decode_timer_off_hours(DaikiriProtocol* protocol) {
    uint64_t mask = 0x3FLL << 40;
    uint8_t code = (uint64_t)((protocol->raw & mask) >> 40);
    protocol->timer_off_hours = (((code & 0xF0LL) >> 4) * 10) + (code & 0x0FLL);
}

void daikiri_protocol_decode_timer_off_minutes(DaikiriProtocol* protocol) {
    uint64_t mask = 1LL << 46;
    uint8_t timer_off_minutes_code = (uint64_t)((protocol->raw & mask) >> 46);
    if(timer_off_minutes_code > 0) {
        protocol->timer_off_minutes = 30;
    } else {
        protocol->timer_off_minutes = 0;
    }
}

uint64_t set_bit(uint64_t value, size_t idx, int8_t bit_value) {
    uint64_t mask = (int64_t)(((uint64_t)(1LL)) << ((uint64_t)(idx)));
    if(bit_value > 0) {
        // Устаналиваем единичку по индексу.
        return (uint64_t)(value | mask);
    } else {
        // ... а тут нолик :)
        return ((uint64_t)(value)) & (~mask);
    }
}

bool is_equial_by_precision(uint32_t a, uint32_t b, uint32_t precision) {
    uint32_t max;
    uint32_t min;
    if(a > b) {
        max = a;
        min = b;
    } else {
        max = b;
        min = a;
    }
    return (max - min) <= precision;
}

void daikiri_protocol_decode(DaikiriProtocol* protocol) {
    daikiri_protocol_decode_current_time_hours(protocol);
    daikiri_protocol_decode_current_time_minutes(protocol);
    daikiri_protocol_decode_temperature(protocol);
    daikiri_protocol_decode_mode(protocol);
    daikiri_protocol_decode_fan_mode(protocol);
    daikiri_protocol_decode_is_sleep_mode(protocol);
    daikiri_protocol_decode_is_swing(protocol);
    daikiri_protocol_decode_is_toggle_power(protocol);
    daikiri_protocol_decode_hash(protocol);
    daikiri_protocol_decode_is_timer_on_enabled(protocol);
    daikiri_protocol_decode_timer_on_hours(protocol);
    daikiri_protocol_decode_timer_on_minutes(protocol);
    daikiri_protocol_decode_is_timer_off_enabled(protocol);
    daikiri_protocol_decode_timer_off_hours(protocol);
    daikiri_protocol_decode_timer_off_minutes(protocol);
}

DaikiriProtocol* daikiri_protocol_interpret_timings(const uint32_t* timings, size_t timings_cnt) {
    // Общий размер - 137 сигналов.
    // Формат сигнала:
    //   PROLOGUE_MARK_1,
    //   PROLOGUE_MARK_2,
    //   PROLOGUE_MARK_1,
    //   PROLOGUE_MARK_2,
    //   BEGIN_FINISH_MARK,
    //   PROLOGUE_MARK_4,
    //   ...
    //   129 сигналов значащих сигналов
    //   ...
    //   EPILOGUE_MARK_1,
    //   BEGIN_FINISH_MARK.

    if(timings_cnt != 137) {
        FURI_LOG_D(
            TAG,
            "Incorrect signal: invalid count of signals. Expected count 137, but actual is %ld",
            (uint32_t) timings_cnt
        );
        return NULL;
    }
    if(!is_equial_by_precision(timings[0], PROLOGUE_MARK_1, 50)) {
        FURI_LOG_D(TAG, "Incorrect signal: timings[0] is not PROLOGUE_MARK_1. Actual is %ld.", (uint32_t) timings[0]);
        return NULL;
    }
    if(!is_equial_by_precision(timings[1], PROLOGUE_MARK_2, 50)) {
        FURI_LOG_D(TAG, "Incorrect signal: timings[1] is not PROLOGUE_MARK_2. Actual is %ld.", (uint32_t) timings[1]);
        return NULL;
    }
    if(!is_equial_by_precision(timings[2], PROLOGUE_MARK_1, 50)) {
        FURI_LOG_D(TAG, "Incorrect signal: timings[2] is not PROLOGUE_MARK_1. Actual is %ld.", (uint32_t) timings[2]);
        return NULL;
    }
    if(!is_equial_by_precision(timings[3], PROLOGUE_MARK_2, 50)) {
        FURI_LOG_D(TAG, "Incorrect signal: timings[3] is not PROLOGUE_MARK_2. Actual is %ld.", (uint32_t) timings[3]);
        return NULL;
    }
    if(!is_equial_by_precision(timings[4], BEGIN_FINISH_MARK, 50)) {
        FURI_LOG_D(TAG, "Incorrect signal: timings[4] is not BEGIN_FINISH_MARK. Actual is %ld.", (uint32_t) timings[4]);
        return NULL;
    }
    if(!is_equial_by_precision(timings[5], PROLOGUE_MARK_4, 50)) {
        FURI_LOG_D(TAG, "Incorrect signal: timings[5] is not PROLOGU_MARK_4. Actual is %ld.", (uint32_t) timings[5]);
        return NULL;
    }

    uint64_t raw = 0;
    size_t raw_idx = 0;
    // Смотрим на значащие сигналы между маркерными, которые идут вначале и в конце.
    for(size_t i = 6; i < 135; i++) {
        if(is_equial_by_precision(timings[i], ZERO, 50)) {
            raw = set_bit(raw, raw_idx++, 0);
        }
        if(is_equial_by_precision(timings[i], ONE, 50)) {
            raw = set_bit(raw, raw_idx++, 1);
        }
    }

    if(raw_idx != 64) {
        FURI_LOG_W(TAG, "Incorrect signal: invalid position %ld after data recognition.", (uint32_t) raw_idx);
        return NULL;
    }

     FuriString* str = furi_string_alloc();
     for(size_t i = 0; i < timings_cnt; i++) {
         furi_string_cat_printf(str, "\n%lu", timings[i]);
     }

     FURI_LOG_I("DaikiriDecoder", "%s", furi_string_get_cstr(str));

    DaikiriProtocol* protocol = malloc(sizeof(DaikiriProtocol));
    protocol->raw = raw;
    return protocol;
}
