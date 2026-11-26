#pragma once

#include "../../core/base.h"
#include "../../core/config.h"
#include "../../settings/models.h"

void do_first_boot_pre_initialization(bool bIgnoreDrivers);

void do_first_boot_initialization();
Model* first_boot_create_default_model();
