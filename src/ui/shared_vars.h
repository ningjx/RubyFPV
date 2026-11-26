#pragma once
#include "../core/base.h"
#include "../core/config.h"
#include "../protocol/radiopackets2.h"
#include "../protocol/radiopackets_rc.h"
#include "../ipc/shared_mem.h"
#include "../ipc/shared_mem_controller_only.h"
#include "../ipc/shared_mem_i2c.h"
#include "../settings/models.h"
#include "../settings/models_list.h"
#include "../protocol/commands.h"
#include "../settings/core_plugins_settings.h"
#include "../settings/ctrl_settings.h"
#include "renderer/render_engine.h"
#include "../settings/plugins_settings.h"
#include "../../sdk/telemetry_info.h"
#include "../../sdk/settings_info.h"
#include "../../sdk/render_engine_ui.h"

#include "popups/popup_commands.h"
#include "popups/popup_camera_params.h"
#include "menu/menu_objects.h"
#include "osd/osd_plugins.h"
#include "shared_vars_ipc.h"
#include "shared_vars_state.h"
#include "shared_vars_osd.h"


extern vehicle_and_telemetry_info_t g_VehicleTelemetryInfo;

extern u32 g_uControllerId;
extern int g_iBootCount;
extern bool g_bDebugState;
extern bool g_bQuit;

extern RenderEngine* g_pRenderEngine;
extern RenderEngineUI* g_pRenderEngineOSDPlugins;

extern ControllerSettings* g_pControllerSettings;

extern u32 g_uVideoRecordingStartTime;
extern bool g_bIsVideoRecording;
extern bool g_bIsVideoProcessing;
extern bool g_bIsVideoPlaying;
extern u32 g_uVideoPlayingTimeMs;
extern u32 g_uVideoPlayingLengthSec;
extern bool g_bIsTestingAdaptiveVideo;

extern int g_iControllerCPUSpeedMhz;
extern int g_iControllerCPULoad;
extern int g_iControllerCPUTemp;
extern u16 g_uControllerCPUFlags;

extern PopupCameraParams* g_pPopupCameraParams;
extern Popup* g_pPopupLooking;
extern Popup* g_pPopupWrongModel;
extern Popup* g_pPopupLinkLost;

extern bool g_bToglleOSDOff;
extern bool g_bToglleStatsOff;
extern bool g_bToglleAllOSDOff;
extern bool g_bFreezeOSD;

extern bool g_bMenuPopupUpdateVehicleShown;
extern type_radio_links_parameters g_LastGoodRadioLinksParams;
extern bool g_bSwitchingRadioLink;

extern bool g_bHasVideoDataOverloadAlarm;
extern bool g_bHasVideoTxOverloadAlarm;
extern Popup* g_pPopupVideoOverloadAlarm;

extern int g_iMustSendCurrentActiveOSDLayoutCounter;

extern CorePluginSettings g_listVehicleCorePlugins[MAX_CORE_PLUGINS_COUNT];
extern int g_iVehicleCorePluginsCount;

extern bool g_bMustNegociateRadioLinksFlag;
extern bool g_bAskedForNegociateRadioLink;
