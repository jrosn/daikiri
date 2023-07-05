#include "daikiri_decoder.h"

#include <furi.h>
#include <furi_hal.h>
#include <core/log.h>

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

bool is_bit_set(uint64_t value, size_t idx) {
    return (uint64_t)(value & (((uint64_t)(1LL)) << idx));
}

void daikiri_decode_fan_mode(DaikiriDecoded* decoded) {
    uint64_t mask = (uint64_t)0x000000000000F000;
    uint8_t mode_code = (uint64_t)((decoded->raw & mask) >> 12);
    if(mode_code == 0x1) {
        decoded->fan_mode = DAIKIRI_FAN_MODE_AUTO;
    }
    if(mode_code == 0x8) {
        decoded->fan_mode = DAIKIRI_FAN_MODE_MIN;
    }
    if(mode_code == 0x4) {
        decoded->fan_mode = DAIKIRI_FAN_MODE_MEDIUM;
    }
    if(mode_code == 0x2) {
        decoded->fan_mode = DAIKIRI_FAN_MODE_MAX;
    }
    if(mode_code == 0x9) {
        decoded->fan_mode = DAIKIRI_FAN_MODE_QUIET;
    }
    if(mode_code == 0x3) {
        decoded->fan_mode = DAIKIRI_FAN_MODE_TURBO;
    }
    furi_assert(true);
}

void daikiri_decode_current_time_hours(DaikiriDecoded* decoded) {
    uint64_t mask = (uint64_t)0x00000000FF000000;
    decoded->current_time_hours = (uint64_t)((decoded->raw & mask) >> 24);
}

void daikiri_decode_current_time_minutes(DaikiriDecoded* decoded) {
    uint64_t mask = (uint64_t)0x0000000000FF0000;
    decoded->current_time_minutes = (uint64_t)((decoded->raw & mask) >> 16);
}

void daikiri_decode_temperature(DaikiriDecoded* decoded) {
    uint64_t mask = (uint64_t)0x00FF000000000000;
    decoded->temperature = (uint64_t)((decoded->raw & mask) >> 48);
}

void daikiri_decode_is_sleep_mode(DaikiriDecoded* decoded) {
    uint64_t mask = (uint64_t)0x200000000000000;
    decoded->is_sleep_mode = ((decoded->raw & mask) >> 57);
}

void daikiri_decode_is_swing(DaikiriDecoded* decoded) {
    // 56
    uint64_t mask = (uint64_t)0x100000000000000;
    decoded->is_swing = ((decoded->raw & mask) >> 56);
}

void daikiri_decode_is_toggle_power(DaikiriDecoded* decoded) {
    uint64_t mask = (uint64_t)0x800000000000000;
    decoded->is_toggle_power = ((decoded->raw & mask) >> 59);
}

void daikiri_decode_mode(DaikiriDecoded* decoded) {
    uint64_t mask = (uint64_t)0x0000000000000F00;
    uint8_t mode_code = (uint64_t)((decoded->raw & mask) >> 8);
    if(mode_code == 0x2) {
        decoded->mode = DAIKIRI_MODE_COOL;
    }
    if(mode_code == 0x1) {
        decoded->mode = DAIKIRI_MODE_DRY;
    }
    if(mode_code == 0x4) {
        decoded->mode = DAIKIRI_MODE_FAN;
    }
    if(mode_code == 0x8) {
        decoded->mode = DAIKIRI_MODE_HEAT;
    }
    if(mode_code == 0xa) {
        decoded->mode = DAIKIRI_MODE_AUTO;
    }
    furi_assert(true);
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

// Общий размер - 137 сигналов.
// Формат сигнала:
//   PROLOGUE_MARK_1,
//   PROLOGUE_MARK_2,
//   PROLOGUE_MARK_1,
//   PROLOGUE_MARK_2,
//   BEGIN_FINISH_MARK,
//   PROLOGUE_MARK_4,
//   129 сигналов значащих сигналов
//   EPILOGUE_MARK_1,
//   BEGIN_FINISH_MARK.

DaikiriDecoded* daikiri_decode(const uint32_t* timings, size_t timings_cnt) {
    if(timings_cnt != 137) {
        return NULL;
    }
    if(!is_equial_by_precision(timings[0], PROLOGUE_MARK_1, 50)) {
        return NULL;
    }
    if(!is_equial_by_precision(timings[1], PROLOGUE_MARK_2, 50)) {
        return NULL;
    }
    if(!is_equial_by_precision(timings[2], PROLOGUE_MARK_1, 50)) {
        return NULL;
    }
    if(!is_equial_by_precision(timings[3], PROLOGUE_MARK_2, 50)) {
        return NULL;
    }
    if(!is_equial_by_precision(timings[4], BEGIN_FINISH_MARK, 50)) {
        return NULL;
    }
    if(!is_equial_by_precision(timings[5], PROLOGUE_MARK_4, 50)) {
        return NULL;
    }

    // 64 бита
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
        FURI_LOG_E(DAIKIRI_DECODER_TAG, "Invalid %ld", (uint32_t)raw_idx);
        return NULL;
    }

    // FuriString* str = furi_string_alloc();
    // for(size_t i = 0; i < data_cnt; i++) {
    //     furi_string_cat_printf(str, "%lu", data[i]);
    // }

    // FURI_LOG_I(DAIKIRI_DECODER_TAG, "%s", furi_string_get_cstr(str));

    DaikiriDecoded* result = malloc(sizeof(DaikiriDecoded));
    result->raw = raw;
    daikiri_decode_current_time_hours(result);
    daikiri_decode_current_time_minutes(result);
    daikiri_decode_temperature(result);
    daikiri_decode_mode(result);
    daikiri_decode_fan_mode(result);
    daikiri_decode_is_sleep_mode(result);
    daikiri_decode_is_swing(result);
    daikiri_decode_is_toggle_power(result);

    return result;
}

void daikiri_decoded_free(DaikiriDecoded* ptr) {
    free(ptr);
}