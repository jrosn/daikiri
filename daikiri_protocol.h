#pragma once

#include <furi.h>
#include <furi_hal.h>
#include <flipper_application.h>

#define PROLOGUE_MARK_2 9737
#define PROLOGUE_MARK_3 9910
#define PROLOGUE_MARK_1 9886
#define BEGIN_FINISH_MARK 4686
#define PROLOGUE_MARK_4 2425
#define ZERO 336
#define ZERO_MARK 399
#define ONE 886
#define ONE_MARK 425
#define EPILOGUE_MARK_1 50
#define DAIKIRI_TIMINGS_CNT 137

typedef enum {
    DAIKIRI_FAN_MODE_MAX,
    DAIKIRI_FAN_MODE_MEDIUM,
    DAIKIRI_FAN_MODE_MIN,
    DAIKIRI_FAN_MODE_AUTO,
    DAIKIRI_FAN_MODE_QUIET,
    DAIKIRI_FAN_MODE_TURBO
} DaikiriFanMode;

typedef enum {
    DAIKIRI_MODE_COOL,
    DAIKIRI_MODE_DRY,
    DAIKIRI_MODE_FAN,
    DAIKIRI_MODE_HEAT,
    DAIKIRI_MODE_AUTO
} DaikiriMode;

typedef struct {
    DaikiriFanMode fan_mode;
    DaikiriMode mode;
    uint64_t raw;

    uint8_t current_time_hours;
    uint8_t current_time_minutes;

    uint8_t temperature;

    uint8_t hash;

    bool is_sleep_mode;
    bool is_swing;
    bool is_toggle_power;

    bool is_timer_on_enabled;
    uint8_t timer_on_hours;
    uint8_t timer_on_minutes;

    bool is_timer_off_enabled;
    uint8_t timer_off_hours;
    uint8_t timer_off_minutes;
} DaikiriProtocol;

bool is_equial_by_precision(uint32_t a, uint32_t b, uint32_t precision);

void daikiri_protocol_free(DaikiriProtocol* ptr);
