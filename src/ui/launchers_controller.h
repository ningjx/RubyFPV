#pragma once
#include "../core/base.h"
#include "../settings/models.h"

void controller_start_i2c();
void controller_stop_i2c();

const char* controller_validate_radio_settings(Model* pModel, u32* pVehicleNICFreq, int* pVehicleNICFlags, int* pVehicleNICRadioFlags, int* pControllerNICFlags, int* pControllerNICRadioFlags);
