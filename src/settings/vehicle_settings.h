#pragma once

#include "../platform/radxa_zero3w/hardware.h"
#include "models.h"

#define VEHICLE_SETTINGS_STAMP_ID "vVII.6"

typedef struct
{
   int iDevRxLoopTimeout;
} VehicleSettings;

int save_VehicleSettings();
int load_VehicleSettings();
void reset_VehicleSettings();
VehicleSettings* get_VehicleSettings();
