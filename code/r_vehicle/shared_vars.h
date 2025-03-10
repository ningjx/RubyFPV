#pragma once
#include "../base/base.h"
#include "../base/config.h"
#include "../base/models.h"
#include "../base/shared_mem.h"
#include "../base/vehicle_rt_info.h"
#include "../base/utils.h"
#include "../radio/radiopackets2.h"
#include "../radio/radiolink.h"
#include "../radio/radiopacketsqueue.h"
#include "processor_tx_audio.h"
#include "processor_tx_video.h"
#include "video_tx_buffers.h"

// Define this to get profile logs about receiving and processing rx packets times
#define PROFILE_RX 1
#define PROFILE_RX_MAX_TIME 10

#define ROUTER_STATE_RUNNING 1
#define ROUTER_STATE_NEEDS_RESTART_VIDEO_CAPTURE (1<<1)

typedef struct
{
   u32 timeLastLogWrongRxPacket;
   int lastReceivedDBM;
   int lastReceivedNoiseDBM;
   int lastReceivedDataRate;
}
type_uplink_rx_info_stats;

extern bool g_bQuit;
extern bool g_bDebug;
extern bool g_bRouterReady;

extern Model* g_pCurrentModel;
extern shared_mem_process_stats* g_pProcessStats;

extern t_packet_queue s_QueueControlPackets;
 
extern int s_fIPCRouterToCommands;
extern int s_fIPCRouterFromCommands;
extern int s_fIPCRouterToTelemetry;
extern int s_fIPCRouterFromTelemetry;
extern int s_fIPCRouterToRC;
extern int s_fIPCRouterFromRC;

extern int s_fInputVideoStream;

extern bool g_bVideoPaused;

extern u16 s_countTXVideoPacketsOutTemp;
extern u16 s_countTXDataPacketsOutTemp;
extern u16 s_countTXCompactedPacketsOutTemp;

// Router
extern bool g_bDeveloperMode;
extern type_u32_couters g_CoutersMainLoop;

extern u32 s_debugVideoBlocksInCount;

extern t_packet_queue g_QueueRadioPacketsOut;
extern VideoTxPacketsBuffer* g_pVideoTxBuffers;
extern ProcessorTxVideo* g_pProcessorTxVideo;
extern ProcessorTxAudio* g_pProcessorTxAudio;
extern int g_iDefaultRouterThreadPriority;

extern bool g_bRadioReinitialized;
extern shared_mem_radio_stats g_SM_RadioStats;
extern shared_mem_radio_stats_rx_hist* g_pSM_HistoryRxStats;
extern shared_mem_radio_stats_rx_hist g_SM_HistoryRxStats;
extern vehicle_runtime_info g_VehicleRuntimeInfo;

extern type_uplink_rx_info_stats g_UplinkInfoRxStats[MAX_RADIO_INTERFACES];

extern t_sik_radio_state g_SiKRadiosState;

extern bool g_bReinitializeRadioInProgress;
extern bool g_bReceivedPairingRequest;
extern bool g_bHasLinkToController;
extern bool g_bHadEverLinkToController;
extern bool g_bHasSentVehicleSettingsAtLeastOnce;
extern bool g_bNegociatingRadioLinks;

extern u32 g_uControllerId;

extern t_packet_header_ruby_telemetry_extended_extra_info_retransmissions g_PHTE_Retransmissions;
extern t_packet_header_vehicle_tx_history g_PHVehicleTxStats;
//To fix
//extern shared_mem_video_link_stats_and_overwrites g_SM_VideoLinkStats;
extern shared_mem_video_link_graphs g_SM_VideoLinkGraphs;
extern shared_mem_dev_video_bitrate_history g_SM_DevVideoBitrateHistory;

//extern shared_mem_video_frames_stats g_VideoInfoStatsCameraOutput;
//extern shared_mem_video_frames_stats* g_pSM_VideoInfoStatsCameraOutput;
//extern shared_mem_video_frames_stats g_VideoInfoStatsRadioOut;
//extern shared_mem_video_frames_stats* g_pSM_VideoInfoStatsRadioOut;

extern int g_iDebugShowKeyFramesAfterRelaySwitch;

extern int g_iGetSiKConfigAsyncResult;
extern int g_iGetSiKConfigAsyncRadioInterfaceIndex;
extern u8 g_uGetSiKConfigAsyncVehicleLinkIndex;

