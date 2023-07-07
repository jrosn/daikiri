#include "daikiri_protocol.h"

#include <furi.h>

void daikiri_protocol_free(DaikiriProtocol* ptr) {
    free(ptr);
}