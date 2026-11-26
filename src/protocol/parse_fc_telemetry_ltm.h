#pragma once

#include "../core/base.h"
#include "../core/config.h"
#include "../settings/models.h"
#include "radiopackets2.h"

bool parse_telemetry_from_fc_ltm( u8* buffer, int length, t_packet_header_fc_telemetry* pphfct, t_packet_header_ruby_telemetry_extended_v6* pPHRTE, u8 vehicleType );
