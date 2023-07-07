#pragma once

#include <furi.h>

#include "daikiri_protocol.h"

void daikiri_protocol_encode_fan_mode(DaikiriProtocol* decoded);

void daikiri_protocol_encode(DaikiriProtocol* protocol);

void daikiri_protocol_constuct_timings(DaikiriProtocol* protocol, uint32_t** timings_out, size_t* timings_cnt_out);