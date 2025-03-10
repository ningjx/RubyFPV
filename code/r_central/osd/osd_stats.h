#pragma once
#include "../../base/base.h"
#include "../../base/config.h"
#include "../../base/shared_mem.h"
#include "../shared_vars.h"

void osd_stats_init();

void _osd_stats_draw_line(float xLeft, float xRight, float y, u32 uFontId, const char* szTextLeft, const char* szTextRight);

float osd_render_stats_video_decode_get_height(int iDeveloperMode, bool bIsSnapshot, shared_mem_radio_stats* pSM_RadioStats, shared_mem_video_stream_stats_rx_processors* pSM_VideoStats, float scale);
float osd_render_stats_video_decode_get_width(int iDeveloperMode, bool bIsSnapshot, shared_mem_radio_stats* pSM_RadioStats, shared_mem_video_stream_stats_rx_processors* pSM_VideoStats, float scale);
float osd_render_stats_video_decode(float xPos, float yPos, int iDeveloperMode, bool bIsSnapshot, shared_mem_radio_stats* pSM_RadioStats, shared_mem_video_stream_stats_rx_processors* pSM_VideoStats, float scale);

float osd_render_stats_video_stream_keyframe_info(float xPos, float yPos);

float osd_render_stats_telemetry_get_height(float scale);
float osd_render_stats_telemetry(float xPos, float yPos, float scale);

float osd_render_stats_audio_decode_get_height();
float osd_render_stats_audio_decode_get_width();
float osd_render_stats_audio_decode(float xPos, float yPos);

float osd_render_stats_rc_get_height(float scale);
float osd_render_stats_rc(float xPos, float yPos, float scale);

float osd_render_stats_flight_end(float scale);
float osd_render_stats_flights(float scale);

float osd_render_stats_efficiency_get_height(float scale);
float osd_render_stats_efficiency(float xPos, float yPos, float scale);

float osd_render_stats_dev(float xPos, float yPos, float scale);

void osd_render_stats_panels();
