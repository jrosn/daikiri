#pragma once

#include <furi.h>

enum DaikiriRemoteCustomEventType {
    DaikiriCustomEventTypeButtonSelected,
    DaikiriCustomEventTypeSendCommand,
};

#pragma pack(push, 1)
typedef union {
    uint32_t packed_value;
    struct {
        uint16_t type;
        int16_t value;
    } content;
} AC_RemoteCustomEvent;
#pragma pack(pop)

static inline uint32_t daikiri_custom_event_pack(uint16_t type, int16_t value) {
    AC_RemoteCustomEvent event = {.content = {.type = type, .value = value}};
    return event.packed_value;
}

static inline void daikiri_custom_event_unpack(uint32_t packed_value, uint16_t* type, int16_t* value) {
    AC_RemoteCustomEvent event = {.packed_value = packed_value};
    if(type) *type = event.content.type;
    if(value) *value = event.content.value;
}

static inline uint16_t ac_remote_custom_event_get_type(uint32_t packed_value) {
    uint16_t type;
    daikiri_custom_event_unpack(packed_value, &type, NULL);
    return type;
}