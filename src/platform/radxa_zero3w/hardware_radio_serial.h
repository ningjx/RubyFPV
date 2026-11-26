#pragma once
#include "../../core/base.h"
#include "../../core/config.h"
#include "hardware_radio.h"
#include "../../ipc/shared_mem.h"

#ifdef __cplusplus
extern "C" {
#endif 


int hardware_radio_serial_parse_and_add_from_serial_ports_config();

int hardware_radio_serial_open_for_read_write(int iHWRadioInterfaceIndex);
int hardware_radio_serial_close(int iHWRadioInterfaceIndex);

int hardware_radio_serial_write_packet(int iHWRadioInterfaceIndex, u8* pData, int iLength);

#ifdef __cplusplus
}  
#endif 