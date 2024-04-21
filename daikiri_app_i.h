#pragma once

#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_stack.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <storage/storage.h>
#include <flipper_format/flipper_format.h>
#include <notification/notification_messages.h>

#include "daikiri_app.h"
#include "daikiri_protocol.h"
#include "scenes/daikiri_scene.h"
#include "views/ac_remote_panel.h"
#include "daikiri_icons.h"

#define TAG "Daikiri"

#define DAIKIRI_APP_SETTINGS APP_DATA_PATH("settings.txt")

// Macroses for debugging purposes
#define PRINTF_BINARY_PATTERN_INT8 "%c%c%c%c%c%c%c%c"
#define PRINTF_BYTE_TO_BINARY_INT8(i)                                                         \
    (((i) & 0x80ll) ? '1' : '0'), (((i) & 0x40ll) ? '1' : '0'), (((i) & 0x20ll) ? '1' : '0'), \
    (((i) & 0x10ll) ? '1' : '0'), (((i) & 0x08ll) ? '1' : '0'), (((i) & 0x04ll) ? '1' : '0'), \
    (((i) & 0x02ll) ? '1' : '0'), (((i) & 0x01ll) ? '1' : '0')
#define PRINTF_BINARY_PATTERN_INT16 \
    PRINTF_BINARY_PATTERN_INT8      \
    PRINTF_BINARY_PATTERN_INT8
#define PRINTF_BYTE_TO_BINARY_INT16(i)    \
    PRINTF_BYTE_TO_BINARY_INT8((i) >> 8), \
    PRINTF_BYTE_TO_BINARY_INT8(i)
#define PRINTF_BINARY_PATTERN_INT32 PRINTF_BINARY_PATTERN_INT16 PRINTF_BINARY_PATTERN_INT16
#define PRINTF_BYTE_TO_BINARY_INT32(i)      \
    PRINTF_BYTE_TO_BINARY_INT16((i) >> 16), \
    PRINTF_BYTE_TO_BINARY_INT16(i)
#define PRINTF_BINARY_PATTERN_INT64 \
    PRINTF_BINARY_PATTERN_INT32     \
    PRINTF_BINARY_PATTERN_INT32
#define PRINTF_BYTE_TO_BINARY_INT64(i)      \
    PRINTF_BYTE_TO_BINARY_INT32((i) >> 32), \
    PRINTF_BYTE_TO_BINARY_INT32(i)


struct DaikiriApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    ViewStack* view_stack;
    ACRemotePanel* ac_remote_panel;
    DaikiriProtocol* state;
};

typedef enum {
    DaikiriAppViewStack,
} DaikiriAppView;