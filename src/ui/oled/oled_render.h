#pragma once
#include "../../core/base.h"
#include "../../core/config.h"
#include "oled_ssd1306.h"

int oled_render_init();
void oled_render_shutdown();
int oled_render_thread_start();