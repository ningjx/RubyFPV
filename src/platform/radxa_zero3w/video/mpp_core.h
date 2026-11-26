#pragma once

#include "../../../core/base.h"
#include "../../../core/config.h"
#include "../hardware.h"
#include "../hardware_procs.h"
#include "../../../ipc/shared_mem.h"
#include "../display/drm_core.h"
#include <ctype.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <semaphore.h>

#include <linux/videodev2.h>
#include <rockchip/rk_mpi.h>

extern shared_mem_process_stats* g_pSMProcessStats;

int mpp_init(bool bUseH265Decoder, int iMPPBuffersSize, u32 uCPUAffinityMask, int iRawPriority);
int mpp_uninit();
void mpp_enable_vsync(bool bEnableVSync);

int mpp_start_decoding_thread();
int mpp_feed_data_to_decoder(void* pData, int iLength);
int mpp_mark_end_of_stream();
bool mpp_get_clear_stream_changed_flag();


