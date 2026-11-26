#pragma once

#include "../base/base.h"
#include "../base/config.h"
#include "../base/models.h"

void do_first_boot_pre_initialization(bool bIgnoreDrivers);

void do_first_boot_initialization();
Model* first_boot_create_default_model();
