#pragma once
#include "frame_clock.h"
#include <functional>

const FrameClock& ui_frame_clock();
void ui_present_frame(const std::function<void()>& draw);
enum class ProgressOverlay { Popups, Commands };
void ui_present_progress(ProgressOverlay overlay);
void ui_clear_frame();
