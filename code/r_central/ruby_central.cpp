/*
    Ruby Licence
    Copyright (c) 2025 Petru Soroaga petrusoroaga@yahoo.com
    All rights reserved.

    Redistribution and/or use in source and/or binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions and/or use of the source code (partially or complete) must retain
        the above copyright notice, this list of conditions and the following disclaimer
        in the documentation and/or other materials provided with the distribution.
        * Redistributions in binary form (partially or complete) must reproduce
        the above copyright notice, this list of conditions and the following disclaimer
        in the documentation and/or other materials provided with the distribution.
        * Copyright info and developer info must be preserved as is in the user
        interface, additions could be made to that info.
        * Neither the name of the organization nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.
        * Military use is not permitted.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE AUTHOR (PETRU SOROAGA) BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <locale.h>
#include <time.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/resource.h>
#include <pthread.h>

#include "ruby_central.h"
#include "../radio/radiolink.h"
#include "../radio/radiopackets2.h"
#include "../base/shared_mem.h"
#include "../base/base.h"
#include "../base/hardware.h"
#include "../base/hardware_audio.h"
#include "../base/hdmi.h"
#include "../base/config.h"
#include "../base/ctrl_settings.h"
#include "../base/ctrl_interfaces.h"
#include "../utils/utils_controller.h"
#include "../base/plugins_settings.h"
#include "../base/vehicle_rt_info.h"
//#include "../base/radio_utils.h"
#include "../base/commands.h"
#include "../base/ruby_ipc.h"
#include "../base/core_plugins_settings.h"
#include "../base/utils.h"
#if defined (HW_PLATFORM_RASPBERRY)
#include "../renderer/render_engine_raw.h"
#endif
#if defined (HW_PLATFORM_RADXA_ZERO3)
#include "../renderer/drm_core.h"
#include "../renderer/render_engine_cairo.h"
#endif

#include "../common/string_utils.h"
#include "../common/strings_table.h"
#include "../common/relay_utils.h"
#include "../common/favorites.h"

#include "colors.h"
#include "osd.h"
#include "osd_common.h"
#include "osd_plugins.h"
#include "osd_widgets.h"
#include "osd_debug_stats.h"
#include "menu.h"
#include "fonts.h"
#include "popup.h"
#include "shared_vars.h"
#include "pairing.h"
#include "link_watch.h"
#include "warnings.h"
#include "keyboard.h"
#include "media.h"
#include "render_commands.h"
#include "handle_commands.h"
#include "menu_confirmation.h"
#include "menu_confirmation_hdmi.h"
#include "ui_alarms.h"
#include "notifications.h"
#include "local_stats.h"
#include "rx_scope.h"
#include "forward_watch.h"
#include "timers.h"
#include "launchers_controller.h"
#include "events.h"
#include "menu_info_booster.h"
#include "menu_confirmation_import.h"
#include "process_router_messages.h"
#include "quickactions.h"
#include "oled/oled_render.h"

u32 s_idBgImage[3];
u32 s_idBgImageMenu[3];
int s_iBgImageIndexPrev = 0;
int s_iBgImageIndex = 0;
u32 s_uTimeLastChangeBgImage = 0;

bool g_bPlayIntro = true;

bool g_bMarkedHDMIReinit = false;
bool g_bIsReinit = false;
bool g_bIsHDMIConfirmation = false;

static int s_iRubyFPS = 0;
static int s_iFPSCount = 0;
static u32 s_iFPSLastTimeCheck = 0;

static int s_iCountRequestsPauseWatchdog = 0;

static u32 s_iMicroTimeMenuRender = 0;
static u32 s_iMicroTimeOSDRender = 0;

static u32 s_TimeLastRender = 0;

static u32 s_TimeCentralInitializationComplete = 0;
static u32 s_TimeLastMenuInput = 0;
static u32 s_TimeLastRecordingStop = 0;

static bool s_bFreezeOSD = false;
static u32 s_uTimeFreezeOSD = 0;

Popup popupNoModel("No vehicle defined or linked to!", 0.2, 0.45, 5);
Popup popupStartup("System starting. Please wait.", 0.05, 0.16, 0);

static char s_szFileHDMIChanged[128];

MenuConfirmationHDMI* s_pMenuConfirmHDMI = NULL;
MenuConfirmationImport* s_pMenuConfirmationImport = NULL;

Popup* ruby_get_startup_popup()
{
   popupStartup.useSmallLines(false);
   return &popupStartup;
}

int ruby_get_start_sequence_step()
{
   return s_StartSequence;
}


void load_resources()
{
   loadAllFonts(true);

   srand(get_current_timestamp_ms());
   s_iBgImageIndexPrev = -1;
   s_iBgImageIndex = rand()%3;
   if ( (s_iBgImageIndex < 0) || (s_iBgImageIndex > 2) )
      s_iBgImageIndex = 0;

   if ( access("res/ruby_bg1.png", R_OK) != -1 )
      s_idBgImage[0] = g_pRenderEngine->loadImage("res/ruby_bg1.png");

   if ( access("res/ruby_bg1_blr.png", R_OK) != -1 )
      s_idBgImageMenu[0] = g_pRenderEngine->loadImage("res/ruby_bg1_blr.png");
   else
      s_idBgImageMenu[0] = s_idBgImage[0];


   if ( access("res/ruby_bg2.png", R_OK) != -1 )
      s_idBgImage[1] = g_pRenderEngine->loadImage("res/ruby_bg2.png");

   if ( access("res/ruby_bg2_blr.png", R_OK) != -1 )
      s_idBgImageMenu[1] = g_pRenderEngine->loadImage("res/ruby_bg2_blr.png");
   else
      s_idBgImageMenu[1] = s_idBgImage[1];


   if ( access("res/ruby_bg3.png", R_OK) != -1 )
      s_idBgImage[2] = g_pRenderEngine->loadImage("res/ruby_bg3.png");

   if ( access("res/ruby_bg3_blr.png", R_OK) != -1 )
      s_idBgImageMenu[2] = g_pRenderEngine->loadImage("res/ruby_bg3_blr.png");
   else
      s_idBgImageMenu[2] = s_idBgImage[2];


   osd_load_resources();
}

void _draw_background()
{
   if ( g_TimeNow > s_uTimeLastChangeBgImage + 40000 )
   {
      s_uTimeLastChangeBgImage = g_TimeNow;
      if ( ! g_bUpdateInProgress )
      {
         int iIndex = s_iBgImageIndex;
         while ( iIndex == s_iBgImageIndex )
         {
            iIndex = rand()%3;
         }
         s_iBgImageIndexPrev = s_iBgImageIndex;
         s_iBgImageIndex = iIndex;
      }
   }

   int iImageId = s_idBgImage[s_iBgImageIndex];
   int iImageIdPrev = s_idBgImage[s_iBgImageIndexPrev];
   
   if ( isMenuOn() )
   {
      iImageId = s_idBgImageMenu[s_iBgImageIndex];
      iImageIdPrev = s_idBgImageMenu[s_iBgImageIndexPrev];
   }

   if ( (g_TimeNow >= s_uTimeLastChangeBgImage) && (g_TimeNow < s_uTimeLastChangeBgImage + 3000) )
   {
      int iAlpha = ((s_uTimeLastChangeBgImage+3000) - g_TimeNow)/12;
      if ( iAlpha < 0 )
         iAlpha = 0;
      if ( iAlpha > 255 )
         iAlpha = 255;
      g_pRenderEngine->drawImage(0, 0, 1,1, iImageIdPrev);
      g_pRenderEngine->drawImageAlpha(0, 0, 1,1, iImageId, 255-iAlpha);
   }
   else
      g_pRenderEngine->drawImage(0, 0, 1,1, iImageId);

   double cc[4] = { 80,30,40,1.0 };

   g_pRenderEngine->setColors(cc);
   float width_text = g_pRenderEngine->textRawWidth(g_idFontMenuLarge, SYSTEM_NAME);
   char szBuff[256];
   getSystemVersionString(szBuff, (SYSTEM_SW_VERSION_MAJOR<<8) | SYSTEM_SW_VERSION_MINOR);
   g_pRenderEngine->drawText(0.91, 0.94, g_idFontMenuLarge, SYSTEM_NAME);
   g_pRenderEngine->drawText(0.915+width_text, 0.94, g_idFontMenuLarge, szBuff);

   bool bNoModel = false;

   if ( (NULL == g_pCurrentModel) ||
        (g_bFirstModelPairingDone && (0 == getControllerModelsCount()) && (0 == getControllerModelsSpectatorCount()) ) ||
        (g_bFirstModelPairingDone && (0 == g_uActiveControllerModelVID) ) )
      bNoModel = true;

   g_pRenderEngine->setGlobalAlfa(1.0);
   //double c[4] = {0,0,0,1};
   //g_pRenderEngine->setColors(c);
   //sprintf(szBuff, "Welcome to %s", SYSTEM_NAME);
   //g_pRenderEngine->drawText(0.42, 0.2, g_idFontMenuLarge, szBuff);
   //g_pRenderEngine->drawText(0.42, 0.24, g_idFontMenuLarge, "Digital FPV System");

   double c[4] = {40,40,40,1};
   g_pRenderEngine->setColors(c);
   sprintf(szBuff, "Welcome to %s Digital FPV System", SYSTEM_NAME);
   g_pRenderEngine->drawText(0.35, 0.1, g_idFontMenuLarge, szBuff);

   double c2[4] = {0,0,0,1};
   g_pRenderEngine->setColors(c2);

   if ( s_StartSequence == START_SEQ_COMPLETED )
   if ( bNoModel )
   {
      if ( 0 == getControllerModelsCount() )
      {
         g_pRenderEngine->drawText(0.32, 0.3, g_idFontMenuLarge, "Info: No vehicle defined!");
         g_pRenderEngine->drawText(0.32, 0.34, g_idFontMenuLarge, "You have no vehicles linked to this controller.");
         g_pRenderEngine->drawText(0.32, 0.37, g_idFontMenuLarge, "Press [Menu] key and then select 'Search' to search for a vehicle to connect to.");
      }
      else if ( ! g_bSearching )
      {
         g_pRenderEngine->drawText(0.32, 0.3, g_idFontMenuLarge, "Info: No vehicle selected!");
         g_pRenderEngine->drawText(0.32, 0.34, g_idFontMenuLarge, "You have no vehicle selected as active.");
         g_pRenderEngine->drawText(0.32, 0.37, g_idFontMenuLarge, "Press [Menu] key and then select 'My Vehicles' to select the vehicle to connect to.");
      }
   }
}


void _render_video_player(u32 timeNow)
{
   char szBuff[1024];

   g_pRenderEngine->startFrame();

   double cColor[] = {0,0,0,0.7};
   g_pRenderEngine->setColors(cColor, 0.9);
   g_pRenderEngine->drawRoundRect(0.02, 0.03, 0.36, 0.1, 0.02);
   g_pRenderEngine->setColors(get_Color_MenuText());

   g_pRenderEngine->setFill(255,255,255,1);
   g_pRenderEngine->setStroke(0,0,0,1);
   g_pRenderEngine->setStrokeSize(1);
   float y = 0.046;

   if ( g_uVideoPlayingTimeMs/1000 > g_uVideoPlayingLengthSec+1 )
   {
      sprintf(szBuff, "Finished.");
      g_pRenderEngine->drawText(0.04, y, g_idFontMenuLarge, szBuff);
   }
   else
   {
      char szVerb[32];
      strcpy(szVerb, "Playing");
      if ( access("/tmp/pausedvr", R_OK) != -1 )
         strcpy(szVerb, "Paused");

      sprintf(szBuff, "%s %02d", szVerb, ((g_uVideoPlayingTimeMs/1000)/60));
      float fWidth = g_pRenderEngine->textWidth(g_idFontMenuLarge, szBuff);
      g_pRenderEngine->drawText(0.04, y, g_idFontMenuLarge, szBuff);

      fWidth += 0.15*g_pRenderEngine->textWidth(g_idFontMenuLarge, "A");

      if ( (g_uVideoPlayingTimeMs/500)%2 )
         g_pRenderEngine->drawText(0.04 + fWidth, y, g_idFontMenuLarge, ":");
      fWidth += 0.5*g_pRenderEngine->textWidth(g_idFontMenuLarge, "A");
      sprintf(szBuff, "%02d / %d:%02d", (g_uVideoPlayingTimeMs/1000)%60, g_uVideoPlayingLengthSec/60, g_uVideoPlayingLengthSec%60);
      g_pRenderEngine->drawText(0.04 + fWidth, y, g_idFontMenuLarge, szBuff);
   }
   sprintf(szBuff, "Press [Menu] for pause/resume or [Back] to stop");
   g_pRenderEngine->drawText(0.04, 0.084, g_idFontMenu, szBuff);  

   g_pRenderEngine->endFrame();
}

void _render_video_background()
{
   if ( g_bPlayIntro )
      return;

   u32 uVehicleIdFullVideo = 0;
   u32 uVehicleSoftwareVersion = 0;
   bool bVehicleHasCamera = true;
   bool bDisplayingRelayedVideo = false;

   if ( NULL != g_pCurrentModel )
   {
      uVehicleIdFullVideo = g_pCurrentModel->uVehicleId;
      uVehicleSoftwareVersion = g_pCurrentModel->sw_version;
      Model* pModel = relay_controller_get_relayed_vehicle_model(g_pCurrentModel);
      if ( NULL != pModel )
      if ( relay_controller_must_display_remote_video(pModel) )
      {
         uVehicleIdFullVideo = pModel->uVehicleId;
         uVehicleSoftwareVersion = pModel->sw_version;
         bDisplayingRelayedVideo = true;
      }
   }

   if ( 0 != uVehicleIdFullVideo )
   {
      bVehicleHasCamera = true;
      t_structure_vehicle_info* pRuntimeInfo = get_vehicle_runtime_info_for_vehicle_id(uVehicleIdFullVideo);
      if ( NULL != pRuntimeInfo )
      if ( pRuntimeInfo->bGotRubyTelemetryInfo )
      if ( ! (pRuntimeInfo->headerRubyTelemetryExtended.flags & FLAG_RUBY_TELEMETRY_VEHICLE_HAS_CAMERA) )
         bVehicleHasCamera = false;

      Model* pModel = findModelWithId(uVehicleIdFullVideo, 60);
      if ( NULL != pModel )
      if ( pModel->iCameraCount <= 0 )
         bVehicleHasCamera = false;
      if ( pModel->b_mustSyncFromVehicle )
         bVehicleHasCamera = true;
      if ( bVehicleHasCamera && link_has_received_videostream(uVehicleIdFullVideo) )
         return;
   }

   g_pRenderEngine->setGlobalAlfa(1.0);

   double c1[4] = {0,0,0,1};
   g_pRenderEngine->setColors(c1);
   g_pRenderEngine->drawRect(0, 0, 1,1 );

   double c[4] = {255,255,255,1};
   g_pRenderEngine->setColors(c);

   char szText[256];

   strcpy(szText, "Waiting for video feed");
   if ( bDisplayingRelayedVideo )
      strcpy(szText, "Waiting for video feed from relayed vehicle");

   if ( g_bFirstModelPairingDone )
   if ( ! bVehicleHasCamera )
   {
      strcpy(szText, "This vehicle has no cameras or video streams");
      if ( g_pCurrentModel->relay_params.isRelayEnabledOnRadioLinkId >= 0 )
      if ( g_pCurrentModel->relay_params.uRelayedVehicleId != 0 )
      {
         Model* pModel = findModelWithId(uVehicleIdFullVideo, 61);
         if ( NULL != pModel )
            sprintf(szText, "%s has no cameras or video streams", pModel->getLongName());
      }
   }

   if ( bVehicleHasCamera )
   {
      static u32 sl_uLastTimeLogWaitVideo = 0;
      if ( g_TimeNow > sl_uLastTimeLogWaitVideo + 4000 )
      {
         sl_uLastTimeLogWaitVideo = g_TimeNow;
         log_line("Waiting for video feed from VID %u", uVehicleIdFullVideo);
      }
   }
   float width_text = g_pRenderEngine->textRawWidth(g_idFontOSDBig, szText);
   g_pRenderEngine->drawText((1.0-width_text)*0.5, 0.45, g_idFontOSDBig, szText);
   g_pRenderEngine->drawText((1.0-width_text)*0.5, 0.45, g_idFontOSDBig, szText);

   if ( (uVehicleSoftwareVersion >>16)  < 262 )
   {
      float fHeight = 1.5*g_pRenderEngine->textHeight(g_idFontOSDBig);
      width_text = g_pRenderEngine->textRawWidth(g_idFontOSDBig, L("Video protocols have changed. You must update your vehicle"));
      g_pRenderEngine->drawText((1.0-width_text)*0.5, 0.45+fHeight, g_idFontOSDBig, L("Video protocols have changed. You must update your vehicle"));
      g_pRenderEngine->drawText((1.0-width_text)*0.5, 0.45+fHeight, g_idFontOSDBig, L("Video protocols have changed. You must update your vehicle"));
   }
}

void _render_background_and_paddings(bool bForceBackground)
{
   if ( g_bPlayIntro )
   {
      char szFile[MAX_FILE_PATH_SIZE];
      strcpy(szFile, FOLDER_RUBY_TEMP);
      strcat(szFile, FILE_TEMP_INTRO_PLAYING);
      if ( access(szFile, R_OK) != -1 )
         return;
      g_bPlayIntro = false;
   }

   bool showBg = true;

   if ( ! g_bSearching )
   if ( g_bIsRouterReady )
   if ( link_has_received_main_vehicle_ruby_telemetry() )
   if ( link_has_received_videostream(0) )
      showBg = false;

   if ( g_bSearching && (!g_bSearchFoundVehicle) )
   {
      showBg = true;
      bForceBackground = true;
   }

   if ( ! pairing_isStarted() )
      showBg = true;
   if ( link_has_received_main_vehicle_ruby_telemetry() && (g_TimeNow < pairing_getStartTime() + 1000) )
   {
      bForceBackground = true;
      showBg = true;
   }
   if ( (!link_has_received_main_vehicle_ruby_telemetry()) && (g_TimeNow < pairing_getStartTime() + 3000) )
   {
      bForceBackground = true;
      showBg = true;
   }
   if ( ! g_bFirstModelPairingDone )
      bForceBackground = true;

   if ( NULL != g_pPopupLooking )
      bForceBackground = true;

   if ( showBg || bForceBackground || (! link_has_received_videostream(0)) )
   {
      if ( bForceBackground || (! pairing_isStarted()) || (g_TimeNow < pairing_getStartTime() + 1000) )
         _draw_background();
      else
         _render_video_background();
   }

   float fScreenAspect = (float)(g_pRenderEngine->getScreenWidth())/(float)(g_pRenderEngine->getScreenHeight());
   if ( (!g_bSearching) || g_bSearchFoundVehicle )
   if ( (NULL != g_pCurrentModel) && (!bForceBackground) )
   {
      double c[4] = {0,0,0,1};
      g_pRenderEngine->setGlobalAlfa(1.0);
      g_pRenderEngine->setColors(c);

      float fVideoAspect = (float)(g_pCurrentModel->video_link_profiles[g_pCurrentModel->video_params.user_selected_video_link_profile].width) / (float)(g_pCurrentModel->video_link_profiles[g_pCurrentModel->video_params.user_selected_video_link_profile].height);
      if ( fVideoAspect > fScreenAspect+0.01 )
      {
         int h = 1 + 0.5*(g_pRenderEngine->getScreenHeight() - g_pRenderEngine->getScreenWidth()/fVideoAspect);
         if ( h > 1 )
         {
            float fHBand = (float)h/(float)g_pRenderEngine->getScreenHeight();
            fHBand += g_pRenderEngine->getPixelHeight();
            g_pRenderEngine->drawRect(0, 0, 1.0, fHBand);
            g_pRenderEngine->drawRect(0,1.0-fHBand, 1.0, fHBand);
         }
      }
      else if ( fVideoAspect < fScreenAspect-0.01 )
      {
         int w = 1 + 0.5*(g_pRenderEngine->getScreenWidth() - g_pRenderEngine->getScreenHeight()*fVideoAspect);
         if ( w > 1 )
         {
            float fWBand = (float)w/(float)g_pRenderEngine->getScreenWidth();
            fWBand += g_pRenderEngine->getPixelWidth();
            g_pRenderEngine->drawRect(0, 0, fWBand, 1.0);
            g_pRenderEngine->drawRect(1.0-fWBand, 0, fWBand, 1.0);
         }
      }
   }
}

void render_all_with_menus(u32 timeNow, bool bRenderMenus, bool bForceBackground, bool bDoInputLoop)
{
   ControllerSettings* pCS = get_ControllerSettings();
   Preferences* p = get_Preferences();

   if ( pCS->iFreezeOSD && s_bFreezeOSD )
      return;

   if ( g_bVideoPlaying )
   {
      _render_video_player(timeNow);
      return;
   }

   g_pRenderEngine->startFrame();
   
   _render_background_and_paddings(bForceBackground);
   
   if ( (!g_bSearching) || g_bSearchFoundVehicle )
   if ( ! bForceBackground )
   if ( s_StartSequence == START_SEQ_COMPLETED || s_StartSequence == START_SEQ_FAILED )
   {
      if ( pairing_isStarted() )
      if ( g_bIsRouterReady )
      if ( g_TimeNow >= g_RouterIsReadyTimestamp + 250 )
      if ( NULL == g_pPopupLooking )
      {
         u32 t = get_current_timestamp_micros();
         osd_render_all();
         t = get_current_timestamp_micros() - t;
         if ( t < 300000 )
            s_iMicroTimeOSDRender = s_iMicroTimeOSDRender*0.8 + t*0.2;
      }
      if ( g_bIsRouterReady )
         alarms_render();
   }

   bool bDevMode = false;
   if ( (NULL != pCS) && (0 != pCS->iDeveloperMode) )
      bDevMode = true;
  
   if ( bDevMode )
   //if ( ! bForceBackground )
   if ( (! g_bToglleAllOSDOff) && (!g_bToglleStatsOff) )
   if ( ! p->iDebugShowFullRXStats )
   if ( ! g_bDebugStats )
   //if ( g_bIsRouterReady )
   {
      char szBuff[64];
      float yPos = osd_getMarginY() + osd_getBarHeight() + osd_getSecondBarHeight() + 0.01*osd_getScaleOSD();
      float xPos = osd_getMarginX() + 0.02*osd_getScaleOSD();
      if ( NULL != g_pCurrentModel && osd_get_current_layout_index() >= 0 && osd_get_current_layout_index() < MODEL_MAX_OSD_PROFILES )
      if ( g_pCurrentModel->osd_params.osd_flags2[osd_get_current_layout_index()] & OSD_FLAG2_LAYOUT_LEFT_RIGHT )
      {
         xPos = osd_getMarginX() + osd_getVerticalBarWidth() + 0.01*osd_getScaleOSD();
         yPos = osd_getMarginY() + 0.01*osd_getScaleOSD();
      }
   
      if ( p->iShowCPULoad )
      {
         osd_set_colors();
         g_pRenderEngine->setFill(0,0,0,0.5);
         g_pRenderEngine->setStroke(0,0,0,0);
         g_pRenderEngine->disableRectBlending();
         g_pRenderEngine->drawRect(xPos, yPos-0.003, 0.46, 0.03);
      }

      osd_set_colors_text(get_Color_Dev());
      if ( (g_TimeNow/500) % 2 )
      {
         float fHeight = g_pRenderEngine->textHeight(g_idFontOSDBig);
         float fWidth = fHeight/g_pRenderEngine->getAspectRatio();
         g_pRenderEngine->setFill(0,0,0,0.5);
         g_pRenderEngine->setStroke(0,0,0,0);
         g_pRenderEngine->disableRectBlending();
         g_pRenderEngine->drawRect(xPos, yPos, fWidth*2, fHeight*1.6);
         osd_set_colors_text(get_Color_Dev());
         float fTWidth = g_pRenderEngine->textWidth(g_idFontOSDBig, "[D]");
         osd_show_value( xPos+fWidth-fTWidth*0.5, yPos+fHeight*0.2, "[D]", g_idFontOSDBig );
      }

      if ( p->iShowCPULoad )
      {
         xPos += 0.02*osd_getScaleOSD();
         yPos += 0.003;
         sprintf(szBuff, "UI FPS: %d", s_iRubyFPS);
         osd_show_value(xPos, yPos, szBuff, g_idFontOSDSmall );

         xPos += 0.05*osd_getScaleOSD();
         sprintf(szBuff, "Menu: %.1f ms/frame", s_iMicroTimeMenuRender/1000.0);
         osd_show_value(xPos, yPos, szBuff, g_idFontOSDSmall );

         xPos += 0.095*osd_getScaleOSD();
         sprintf(szBuff, "OSD: %.1f ms/frame", s_iMicroTimeOSDRender/1000.0);
         osd_show_value(xPos, yPos, szBuff, g_idFontOSDSmall );

         xPos += 0.095*osd_getScaleOSD();
         sprintf(szBuff, "OSD: %d ms/sec", (int)(s_iMicroTimeOSDRender*s_iRubyFPS/1000.0));
         osd_show_value(xPos, yPos, szBuff, g_idFontOSDSmall );
      }
      g_pRenderEngine->enableRectBlending();
   }

   u32 t = get_current_timestamp_micros();
   popups_render();
   if ( bRenderMenus )
      menu_render();
   popups_render_topmost();

   t = get_current_timestamp_micros() - t;
   if ( t < 300000 )
      s_iMicroTimeMenuRender = (s_iMicroTimeMenuRender*8 + t*2)/10;
  
   if ( handle_commands_is_command_in_progress() )
      render_commands();

   s_iFPSCount++;
   if ( timeNow >= s_iFPSLastTimeCheck + 1000 )
   {
      s_iRubyFPS = s_iFPSCount;
      s_iFPSCount = 0;
      s_iFPSLastTimeCheck = timeNow;
   }
   
   if ( NULL != p && p->iOSDFlipVertical )
      g_pRenderEngine->rotate180();

   g_pRenderEngine->endFrame();
}

void render_all(u32 timeNow, bool bForceBackground, bool bDoInputLoop)
{
   render_all_with_menus(timeNow, true, bForceBackground, bDoInputLoop);
}


void* _thread_check_controller_cpu_state(void *argument)
{
   log_line("Started thread to check CPU state...");
   g_uControllerCPUFlags = hardware_get_flags();
   g_iControllerCPUSpeedMhz = hardware_get_cpu_speed();

   int temp = 0;
   FILE* fd = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
   if ( NULL != fd )
   {
      fscanf(fd, "%d", &temp);
      fclose(fd);
      fd = NULL;
   }
   g_iControllerCPUTemp = temp/1000;
   log_line("Finished thread to check CPU state.");
   return NULL;
}

void compute_cpu_state()
{
   static u32 s_TimeLastCPUComputeLoad = 0;
   static u32 s_TimeLastCPUComputeState = 0;

   if ( g_TimeNow > s_TimeLastCPUComputeLoad + 1000 )
   {
      s_TimeLastCPUComputeLoad = g_TimeNow;

      FILE* fd = NULL;
      static unsigned long long valgcpu[4] = {0,0,0,0};
      unsigned long long tmp[4];
      unsigned long long total;

      fd = fopen("/proc/stat", "r");
      if ( NULL != fd )
      {
         fscanf(fd, "%*s %llu %llu %llu %llu", &tmp[0], &tmp[1], &tmp[2], &tmp[3] );
         fclose(fd);
         fd = NULL;
      }
      else
      {
          tmp[0] = tmp[1] = tmp[2] = tmp[3] = 0;
      }
      
      if ( tmp[0] < valgcpu[0] || tmp[1] < valgcpu[1] || tmp[2] < valgcpu[2] || tmp[3] < valgcpu[3] )
            g_iControllerCPULoad = 0;
      else
      {
         total = (tmp[0] - valgcpu[0]) + (tmp[1] - valgcpu[1]) + (tmp[2] - valgcpu[2]);
         if ( (total + (tmp[3] - valgcpu[3])) != 0 )
            g_iControllerCPULoad = (total * 100) / (total + (tmp[3] - valgcpu[3]));
      }
         
      valgcpu[0] = tmp[0]; valgcpu[1] = tmp[1]; valgcpu[2] = tmp[2]; valgcpu[3] = tmp[3]; 
   }

   if ( g_TimeNow > s_TimeLastCPUComputeState + 10000 )
   {
      s_TimeLastCPUComputeState = g_TimeNow;
      pthread_t pth;
      pthread_create(&pth, NULL, &_thread_check_controller_cpu_state, NULL);
   }
}

int ruby_start_recording()
{
   if ( NULL == g_pCurrentModel )
      return -1;
   if ( g_bVideoRecordingStarted )
      return -1;
   if ( (g_TimeNow < s_TimeLastRecordingStop + 1000) || g_bVideoProcessing )
   {
      Popup* p = new Popup("Please wait for the current video file processing to complete.", 0.1,0.7, 0.54, 5);
      p->setIconId(g_idIconError, get_Color_IconError());
      popups_add_topmost(p);
      return -1;
   }

   char szBuff[1024];
   char szComm[256];
   #ifdef HW_PLATFORM_RASPBERRY
   system("sudo mount -o remount,rw /");
   #endif
   
   hw_execute_bash_command("mkdir -p tmp", NULL );
   hw_execute_bash_command("chmod 777 tmp", NULL );

   sprintf(szBuff, "rm -rf %s%s 2>/dev/null", FOLDER_RUBY_TEMP, FILE_TEMP_VIDEO_FILE_PROCESS_ERROR);
   hw_execute_bash_command(szBuff, NULL );

   if ( access( FOLDER_MEDIA, R_OK ) == -1 )
   {
      sprintf(szBuff, "mkdir -p %s",FOLDER_MEDIA);
      hw_execute_bash_command(szBuff, NULL );
      sprintf(szBuff, "chmod 777 %s",FOLDER_MEDIA);
      hw_execute_bash_command(szBuff, NULL );

      sprintf(szBuff, "chmod 777 %s* 2>/dev/null", FOLDER_MEDIA);
      hw_execute_bash_command(szBuff, NULL);

      if ( access( FOLDER_MEDIA, R_OK ) == -1 )
      {
         Popup* p = new Popup("There is an issue writing to the SD card.", 0.1,0.7, 0.54, 5);
         p->setIconId(g_idIconError, get_Color_IconError());
         popups_add_topmost(p);
         return -1;
      }
   }

   g_uVideoRecordStartTime = get_current_timestamp_ms();

   #ifdef HW_PLATFORM_RASPBERRY
   sprintf(szComm, "df -m %s | grep root", FOLDER_BINARIES);
   #endif
   #ifdef HW_PLATFORM_RADXA_ZERO3
   sprintf(szComm, "df -m %s | grep mmc", FOLDER_BINARIES);
   #endif
   if ( 1 == hw_execute_bash_command_raw(szComm, szBuff) )
   {
      char szTemp[1024];
      long lb, lu, lf;
      sscanf(szBuff, "%s %ld %ld %ld", szTemp, &lb, &lu, &lf);
      if ( lf < 200 )
      {
         sprintf(szTemp, "You don't have enough free space on the SD card to start recording (%d Mb free). Move your media files to USB memory stick.", (int)lf);
         warnings_add(0, szTemp, g_idIconCamera, get_Color_IconError(), 6);
         return -1;
      }
      if ( lf < 1000 )
      {
         sprintf(szTemp, "You are running low on storage space (%d Mb free). Move your media files to USB memory stick.", (int)lf);
         warnings_add(0, szTemp, g_idIconCamera, get_Color_IconWarning(), 6);
         log_line("Warning: Free storage space is only %d Mb. Video recording might stop", (int)lf);
      }
   }

   sem_t* s = sem_open(SEMAPHORE_START_VIDEO_RECORD, O_CREAT, S_IWUSR | S_IRUSR, 0);
   sem_post(s);
   sem_close(s); 

   Preferences* p = get_Preferences();
   if ( p->iRecordingLedAction == 1 )
      hardware_recording_led_set_on();
   if ( p->iRecordingLedAction == 2 )
      hardware_recording_led_set_blinking();
   g_bVideoRecordingStarted = true;
   
   notification_add_recording_start();

   t_packet_header PH;
   radio_packet_init(&PH, PACKET_COMPONENT_RUBY, PACKET_TYPE_VEHICLE_RECORDING, STREAM_ID_DATA);
   PH.vehicle_id_src = g_uControllerId;
   PH.vehicle_id_dest = g_pCurrentModel->uVehicleId;
   PH.total_length = sizeof(t_packet_header) + 8 * sizeof(u8);

   u8 buffer[MAX_PACKET_TOTAL_SIZE];
   memset(buffer, 0, MAX_PACKET_TOTAL_SIZE);
   memcpy(buffer, (u8*)&PH, sizeof(t_packet_header));
   buffer[sizeof(t_packet_header)] = 1; // Start audio recording
   send_packet_to_router(buffer, PH.total_length);
   return 0;
}

int ruby_stop_recording()
{
   if ( g_TimeNow < g_uVideoRecordStartTime + 200 )
      return 0;
   s_TimeLastRecordingStop = g_TimeNow;

   if ( NULL == g_pCurrentModel )
      return -1;
   if ( ! g_bVideoRecordingStarted )
      return -1;
   sem_t* s = sem_open(SEMAPHORE_STOP_VIDEO_RECORD, O_CREAT, S_IWUSR | S_IRUSR, 0);
   sem_post(s);
   sem_close(s); 
   g_bVideoRecordingStarted = false;

   hardware_recording_led_set_off();

   notification_add_recording_end();
   g_bVideoProcessing = true;
   link_watch_mark_started_video_processing();
   warnings_add(0, "Processing video file...", g_idIconCamera, get_Color_IconNormal());

   t_packet_header PH;
   radio_packet_init(&PH, PACKET_COMPONENT_RUBY, PACKET_TYPE_VEHICLE_RECORDING, STREAM_ID_DATA);
   PH.vehicle_id_src = g_uControllerId;
   PH.vehicle_id_dest = g_pCurrentModel->uVehicleId;
   PH.total_length = sizeof(t_packet_header) + 8 * sizeof(u8);

   u8 buffer[MAX_PACKET_TOTAL_SIZE];
   memset(buffer, 0, MAX_PACKET_TOTAL_SIZE);
   memcpy(buffer, (u8*)&PH, sizeof(t_packet_header));
   buffer[sizeof(t_packet_header)] = 2; // Stop audio recording
   send_packet_to_router(buffer, PH.total_length);
   return 0;
}

int exectuteActionsInputDebugStats()
{
   Preferences* p = get_Preferences();
   if ( NULL == p )
      return 0;
   if ( g_bIsReinit || g_bSearching )
      return 0;
   if ( (!pairing_isStarted()) || (! g_bIsRouterReady) )
      return 0;

   int iRet = 0;

   if ( p->iDebugStatsQAButton == 1 )
   if ( keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA1 )
   {
      g_bDebugStats = ! g_bDebugStats;
      iRet = 1;
   }

   if ( p->iDebugStatsQAButton == 2 )
   if ( keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA2 )
   {
      g_bDebugStats = ! g_bDebugStats;
      iRet = 1;
   }

   if ( p->iDebugStatsQAButton == 3 )
   if ( keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA3 )
   {
      g_bDebugStats = ! g_bDebugStats;
      iRet = 1;
   }

   if ( (keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_BACK) )
   if ( ! isMenuOn() )
   {
      g_bDebugStats = false;
      iRet = 1;
   }

   if ( g_bDebugStats )
   if ( ! isMenuOn() )
   if ( ((p->iDebugStatsQAButton != 1) && (keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA1)) ||
        ((p->iDebugStatsQAButton != 2) && (keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA2)) ||
        ((p->iDebugStatsQAButton != 3) && (keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA3)) )
   {
      osd_debug_stats_toggle_freeze();
      iRet = 1;
   }

   if ( g_bDebugStats )
   if ( (keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_MINUS) ||
        (keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_PLUS) )
   if ( ! isMenuOn() )
   {
      osd_debug_stats_toggle_zoom();
      iRet = 1;
   }
   return iRet;
}

void executeQuickActions()
{
   ControllerSettings* pCS = get_ControllerSettings();
   Preferences* p = get_Preferences();
   if ( NULL == p )
      return;
   if ( g_bIsReinit || g_bSearching )
      return;
   if ( NULL == g_pCurrentModel )
   {
      Popup* p = new Popup("You must be connected to a vehicle to execute Quick Actions.", 0.1,0.8, 0.54, 5);
      p->setIconId(g_idIconError, get_Color_IconError());
      popups_add_topmost(p);
      return;
   }

   if ( keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA1  )
      log_line("Pressed button QA1");
   if ( keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA2  )
      log_line("Pressed button QA2");
   if ( keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA3  )
      log_line("Pressed button QA3");

   log_line("Current assigned QA actions: button1: %d, button2: %d, button3: %d",
    p->iActionQuickButton1,p->iActionQuickButton2,p->iActionQuickButton3);
   
   if ( (!pairing_isStarted()) || (! g_bIsRouterReady) )
   {
      warnings_add(0, "Please connect to a vehicle first, to execute Quick Actions.");
      return;
   }

   if ( pCS->iDevSwitchVideoProfileUsingQAButton >= 0 && pCS->iDevSwitchVideoProfileUsingQAButton < 3 )
   if ( ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA1) && (pCS->iDevSwitchVideoProfileUsingQAButton==0)) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA2) && (pCS->iDevSwitchVideoProfileUsingQAButton==1)) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA3) && (pCS->iDevSwitchVideoProfileUsingQAButton==2)) )
   {
      executeQuickActionSwitchVideoProfile();
      return;
   }

   if ( ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA1) && quickActionCycleOSD == p->iActionQuickButton1) || 
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA2) && quickActionCycleOSD == p->iActionQuickButton2) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA3) && quickActionCycleOSD == p->iActionQuickButton3) )
   {
      executeQuickActionCycleOSD();
      return;
   }

   if ( ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA1) && quickActionOSDSize == p->iActionQuickButton1) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA2) && quickActionOSDSize == p->iActionQuickButton2) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA3) && quickActionOSDSize == p->iActionQuickButton3) )
   {
      if ( ! quickActionCheckVehicle("change OSD size") )
         return;
      p->iScaleOSD++;
      if ( p->iScaleOSD > 3 )
         p->iScaleOSD = -1;
      save_Preferences();
      osd_apply_preferences();
      return;
   }

   if ( ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA1) && quickActionTakePicture == p->iActionQuickButton1) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA2) && quickActionTakePicture == p->iActionQuickButton2) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA3) && quickActionTakePicture == p->iActionQuickButton3) )
   {
      executeQuickActionTakePicture();
      return;
   }
         
   if ( ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA1) && quickActionToggleOSD == p->iActionQuickButton1) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA2) && quickActionToggleOSD == p->iActionQuickButton2) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA3) && quickActionToggleOSD == p->iActionQuickButton3) )
   {

      if ( ! quickActionCheckVehicle("toggle the OSD") )
         return;
      g_bToglleOSDOff = ! g_bToglleOSDOff;
      return;
   }

   if ( ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA1) && quickActionToggleStats == p->iActionQuickButton1) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA2) && quickActionToggleStats == p->iActionQuickButton2) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA3) && quickActionToggleStats == p->iActionQuickButton3) )
   {
      if ( ! quickActionCheckVehicle("toggle the statistics") )
         return;
      g_bToglleStatsOff = ! g_bToglleStatsOff;
      return;
   }
         
   if ( ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA1) && quickActionToggleAllOff == p->iActionQuickButton1) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA2) && quickActionToggleAllOff == p->iActionQuickButton2) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA3) && quickActionToggleAllOff == p->iActionQuickButton3) )
   {
      if ( ! quickActionCheckVehicle("toggle all info on/off") )
         return;
      g_bToglleAllOSDOff = ! g_bToglleAllOSDOff;
      return;
   }

   if ( ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA1) && quickActionOSDFreeze == p->iActionQuickButton1) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA2) && quickActionOSDFreeze == p->iActionQuickButton2) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA3) && quickActionOSDFreeze == p->iActionQuickButton3) )
   {
      g_bFreezeOSD = ! g_bFreezeOSD;
      return;
   }

   if ( ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA1) && quickActionRelaySwitch == p->iActionQuickButton1) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA2) && quickActionRelaySwitch == p->iActionQuickButton2) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA3) && quickActionRelaySwitch == p->iActionQuickButton3) )
   {
      executeQuickActionRelaySwitch();
      return;
   }

   if ( ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA1) && quickActionVideoRecord == p->iActionQuickButton1) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA2) && quickActionVideoRecord == p->iActionQuickButton2) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA3) && quickActionVideoRecord == p->iActionQuickButton3) )
   {
      executeQuickActionRecord();
      return;
   }

   #ifdef FEATURE_ENABLE_RC
   if ( NULL != g_pCurrentModel )
   if ( ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA1) && quickActionRCEnable == p->iActionQuickButton1) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA2) && quickActionRCEnable == p->iActionQuickButton2) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA3) && quickActionRCEnable == p->iActionQuickButton3) )
   {
      if ( (NULL != g_pCurrentModel) && g_pCurrentModel->is_spectator )
      {
         warnings_add(0, "Can't enable RC while in spectator mode.");
         return;
      }
      if ( ! quickActionCheckVehicle("enable/disable the RC link output") )
         return;

      rc_parameters_t params;
      memcpy(&params, &g_pCurrentModel->rc_params, sizeof(rc_parameters_t));

      if ( params.flags & RC_FLAGS_OUTPUT_ENABLED )
         params.flags &= (~RC_FLAGS_OUTPUT_ENABLED);
      else
         params.flags |= RC_FLAGS_OUTPUT_ENABLED;
      handle_commands_abandon_command();
      handle_commands_send_to_vehicle(COMMAND_ID_SET_RC_PARAMS, 0, (u8*)&params, sizeof(rc_parameters_t));
      return;
   }
   #endif

   if ( ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA1) && quickActionCameraProfileSwitch == p->iActionQuickButton1) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA2) && quickActionCameraProfileSwitch == p->iActionQuickButton2) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA3) && quickActionCameraProfileSwitch == p->iActionQuickButton3) )
   {
      if ( g_pCurrentModel->is_spectator )
      {
         warnings_add(0, "Can't switch camera profile for spectator vehicles.");
         return;
      }
      if ( handle_commands_is_command_in_progress() )
      {
         return;
      }

      int iProfile = g_pCurrentModel->camera_params[g_pCurrentModel->iCurrentCamera].iCurrentProfile;
      camera_profile_parameters_t* pProfile1 = &(g_pCurrentModel->camera_params[g_pCurrentModel->iCurrentCamera].profiles[iProfile]);
      iProfile++;
      if ( iProfile >= MODEL_CAMERA_PROFILES-1 )
         iProfile = 0;

      camera_profile_parameters_t* pProfile2 = &(g_pCurrentModel->camera_params[g_pCurrentModel->iCurrentCamera].profiles[iProfile]);
      
      //char szBuff[64];
      //sprintf(szBuff, "Switching to camera profile %s", model_getCameraProfileName(iProfile));
      //warnings_add(g_pCurrentModel->uVehicleId, szBuff);

      log_camera_profiles_differences(pProfile1, pProfile2);

      handle_commands_send_to_vehicle(COMMAND_ID_SET_CAMERA_PROFILE, iProfile, NULL, 0);
      return;
   }

   if ( ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA1) && quickActionRotaryFunction == p->iActionQuickButton1) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA2) && quickActionRotaryFunction == p->iActionQuickButton2) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA3) && quickActionRotaryFunction == p->iActionQuickButton3) )
   {
      pCS->nRotaryEncoderFunction++;
      if ( pCS->nRotaryEncoderFunction > 2 )
         pCS->nRotaryEncoderFunction = 1;
      save_ControllerSettings();
      if ( 0 == pCS->nRotaryEncoderFunction )
         warnings_add(0, "Rotary Encoder function changed to: None");
      if ( 1 == pCS->nRotaryEncoderFunction )
         warnings_add(0, "Rotary Encoder function changed to: Menu Navigation");
      if ( 2 == pCS->nRotaryEncoderFunction )
         warnings_add(0, "Rotary Encoder function changed to: Camera Adjustment");

      return;
   }

   if ( ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA1) && quickActionSwitchFavorite == p->iActionQuickButton1) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA2) && quickActionSwitchFavorite == p->iActionQuickButton2) ||
        ((keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_QA3) && quickActionSwitchFavorite == p->iActionQuickButton3) )
   {
      executeQuickActionSwitchFavoriteVehicle();
      return;
   }
}


void r_check_processes_filesystem()
{
   char szFilesMissing[1024];
   szFilesMissing[0] = 0;
   bool failed = false;
   if( access( "ruby_rt_station", R_OK ) == -1 )
      { failed = true; strcat(szFilesMissing, " ruby_rt_station"); }
   if( access( "ruby_rx_telemetry", R_OK ) == -1 )
      { failed = true; strcat(szFilesMissing, " ruby_rx_telemetry"); }
   if( access( "ruby_video_proc", R_OK ) == -1 )
      { failed = true; strcat(szFilesMissing, " ruby_video_proc"); }
   if( access( VIDEO_PLAYER_SM, R_OK ) == -1 )
      { failed = true; strcat(szFilesMissing, " "); strcat(szFilesMissing, VIDEO_PLAYER_SM); }
   if( access( VIDEO_PLAYER_OFFLINE, R_OK ) == -1 )
      { failed = true; strcat(szFilesMissing, " "); strcat(szFilesMissing, VIDEO_PLAYER_OFFLINE); }
     
   log_line("Checked proccesses. Result: %s", failed?"failed":"all ok");

   if ( failed )
   {
      log_error_and_alarm("Some Ruby binaries are missing: [%s].", szFilesMissing);

      Popup* p = new Popup("Some processes are missing from the SD card!", 0.2, 0.65, 0.6, 0);
      p->setCentered();
      p->setIconId(g_idIconError,get_Color_IconError());
      popups_add(p);
   }
}

void ruby_load_models()
{
   log_line("Loading models...");
   loadAllModels();
   g_pCurrentModel = getCurrentModel();
   log_line("Loaded models complete.");
   
   char szFile[128];
   strcpy(szFile, FOLDER_CONFIG);
   strcat(szFile, FILE_CONFIG_ACTIVE_CONTROLLER_MODEL);
   if ( access(szFile, R_OK) == -1 )
   {
      if ( g_bFirstModelPairingDone )
      if ( (0 != getControllerModelsCount()) || ( 0 != getControllerModelsSpectatorCount()) )
      if ( NULL != g_pCurrentModel )
          g_uActiveControllerModelVID = g_pCurrentModel->uVehicleId;

      if ( ! controllerHasModelWithId(g_uActiveControllerModelVID) )
         g_uActiveControllerModelVID = 0;

      // Recreate active model file
      ruby_set_active_model_id(g_uActiveControllerModelVID);
   }
   FILE* fd = fopen(szFile, "rb");
   if ( NULL != fd )
   {
      fscanf(fd, "%u", &g_uActiveControllerModelVID);
      fclose(fd);
      log_line("Controller current active model id is: %u", g_uActiveControllerModelVID);
   }
   else
      log_softerror_and_alarm("Can't access active model id file (%s)", szFile);

   if ( ! controllerHasModelWithId(g_uActiveControllerModelVID) )
   {
      log_line("Controller does not have a model for current active controller model id %u. Reset active model id to 0.", g_uActiveControllerModelVID);
      ruby_set_active_model_id(0);
   }
   if ( g_bFirstModelPairingDone )
   {
      if ( (0 == getControllerModelsCount()) && ( 0 == getControllerModelsSpectatorCount()) )
      {
         log_line("No controller or spectator models and first pairing was done. No active model to use.");
         ruby_set_active_model_id(0);
         return;
      }
   }

   log_line("Current model is in %s mode", g_pCurrentModel->is_spectator?"spectator":"controller");
   char szFreq1[64];
   char szFreq2[64];
   char szFreq3[64];
   strcpy(szFreq1, str_format_frequency(g_pCurrentModel->radioLinksParams.link_frequency_khz[0]));
   strcpy(szFreq2, str_format_frequency(g_pCurrentModel->radioLinksParams.link_frequency_khz[1]));
   strcpy(szFreq3, str_format_frequency(g_pCurrentModel->radioLinksParams.link_frequency_khz[2]));
   
   log_line("Current model radio links: %d, 1st radio link frequency: %s, 2nd radio link: %s, 3rd radio link: %s",
      g_pCurrentModel->radioLinksParams.links_count, 
      szFreq1, szFreq2, szFreq3);
}

void _on_start_completed()
{
   char szComm[256];
   char szFile[MAX_FILE_PATH_SIZE];
   strcpy(szFile, FOLDER_RUBY_TEMP);
   strcat(szFile, FILE_TEMP_INTRO_PLAYING);

   if ( access(szFile, R_OK) != -1 )
   {
      log_line("Intro video is playing. Stopping it...");
      hw_stop_process(VIDEO_PLAYER_OFFLINE);
      snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "rm -rf %s", szFile);
      hw_execute_bash_command(szComm, NULL);
   }
   g_bPlayIntro = false;
}

void start_loop()
{
   hardware_sleep_ms(START_SEQ_DELAY);
   log_line("Executing start up sequence step: %d", s_StartSequence);
   if ( s_StartSequence == START_SEQ_PRE_LOAD_CONFIG )
   {
      log_line("Loading configuration...");
      popupStartup.setTitle(SYSTEM_NAME);
      if ( g_bIsReinit )
         popupStartup.addLine("Restarting...");
      
      popupStartup.addLine("Loading configuration...");
      log_line("Finished executing start up sequence step: %d", s_StartSequence);
      s_StartSequence = START_SEQ_LOAD_CONFIG;
      return;
   }
   if ( s_StartSequence == START_SEQ_LOAD_CONFIG )
   {
      if ( ! load_Preferences() )
         save_Preferences();
      osd_apply_preferences();

      load_PluginsSettings();
      load_CorePluginsSettings();
      load_CorePlugins(1);
      if ( access("/sys/class/net/usb0", R_OK ) == -1 )
      {
         char szBuff[256];
         sprintf(szBuff, "rm -rf %s%s", FOLDER_RUBY_TEMP, FILE_TEMP_USB_TETHERING_DEVICE);
         hw_execute_bash_command(szBuff, NULL);
      }

      log_line("Finished executing start up sequence step: %d", s_StartSequence);
      s_StartSequence = START_SEQ_PRE_SEARCH_DEVICES;
      return;
   }

   if ( s_StartSequence == START_SEQ_PRE_SEARCH_DEVICES )
   {
      popupStartup.addLine("Searching for external devices...");
      s_StartSequence = START_SEQ_SEARCH_DEVICES;
      return;
   }

   if ( s_StartSequence == START_SEQ_SEARCH_DEVICES )
   {
      hardware_i2c_check_and_update_device_settings();
      ruby_signal_alive();
      controller_stop_i2c();
      controller_start_i2c();
      ruby_signal_alive();
      hardware_sleep_ms(200);

      s_StartSequence = START_SEQ_PRE_SEARCH_INTERFACES;
      return;
   }

   if ( s_StartSequence == START_SEQ_PRE_SEARCH_INTERFACES )
   {
      log_line("Enumerating controller input interfaces...");
      popupStartup.addLine("Enumerating controller input interfaces...");
      s_StartSequence = START_SEQ_SEARCH_INTERFACES;
      return;
   }

   if ( s_StartSequence == START_SEQ_SEARCH_INTERFACES )
   {
      controllerInterfacesEnumJoysticks();
      s_StartSequence = START_SEQ_PRE_NICS;
      return;
   }

   if ( s_StartSequence == START_SEQ_PRE_NICS )
   {
      log_line("Getting radio hardware info...");
      popupStartup.addLine("Getting radio hardware info...");
      hardware_enumerate_radio_interfaces();

      log_line("Finished executing start up sequence step: %d", s_StartSequence);
      s_StartSequence = START_SEQ_NICS;
      return;
   }


   if ( s_StartSequence == START_SEQ_NICS )
   {
     if ( 0 == hardware_get_radio_interfaces_count() )
     {
         Popup* p = new Popup("No radio interfaces detected on your controller.",0.3,0.4, 0.5, 6);
         p->setIconId(g_idIconError, get_Color_IconError());
         popups_add_topmost(p);
     }
     else
     {
        bool allDisabled = true;
        for( int i=0; i<hardware_get_radio_interfaces_count(); i++ )
        {
           radio_hw_info_t* pNIC = hardware_get_radio_info(i);
           u32 flags = controllerGetCardFlags(pNIC->szMAC);
           if ( ! ( flags & RADIO_HW_CAPABILITY_FLAG_DISABLED ) )
              allDisabled = false;
        }
        if ( allDisabled )
        {
           Popup* p = new Popup("All radio interfaces are disabled on the controller.", 0.3,0.4, 0.5, 6);
           p->setIconId(g_idIconError, get_Color_IconError());
           p->addLine("Go to [Menu]->[Controller] -> [Radio Configuration] and enable at least one radio interface.");
           popups_add_topmost(p);
           log_line("Finished executing start up sequence step: %d", s_StartSequence);
           s_StartSequence = START_SEQ_PRE_SYNC_DATA;
           return;
        }

        int countUnsuported = 0;
        for( int i=0; i<hardware_get_radio_interfaces_count(); i++ )
        {
           radio_hw_info_t* pNIC = hardware_get_radio_info(i);
           if ( 0 == pNIC->isSupported )
              countUnsuported++;
        }
        if ( countUnsuported == hardware_get_radio_interfaces_count() )
        {
            Popup* p = new Popup("No radio interface on your controller is fully supported.", 0.3,0.4, 0.5, 6);
            p->setIconId(g_idIconError, get_Color_IconError());
            popups_add_topmost(p);
        }
        else if ( countUnsuported > 0 )
        {
            Popup* p = new Popup("Some radio interfaces on your controller are not fully supported.", 0.3,0.4, 0.5, 6);
            p->setIconId(g_idIconWarning, get_Color_IconWarning());
            popups_add_topmost(p);
        }

        int iNewCardIndex = controllerAddedNewRadioInterfaces();
        if ( iNewCardIndex >= 0 )
        {
           save_ControllerInterfacesSettings();
           if ( g_bFirstModelPairingDone )
           {
              MenuInfoBooster* pMenu = new MenuInfoBooster();
              if ( NULL != pMenu )
              {
                 pMenu->m_iRadioCardIndex = iNewCardIndex;
                 add_menu_to_stack(pMenu);
              }
           }
        }

        // Check & update radio cards models
        bool bChanged = false;
        ControllerInterfacesSettings* pCIS = get_ControllerInterfacesSettings();
        for( int i=0; i<pCIS->radioInterfacesCount; i++ )
        {
            // If user defined, do not change it
            if ( pCIS->listRadioInterfaces[i].cardModel < 0 )
               continue;
            radio_hw_info_t* pNICInfo = hardware_get_radio_info_from_mac(pCIS->listRadioInterfaces[i].szMAC);
            if ( NULL != pNICInfo )
            if ( pCIS->listRadioInterfaces[i].cardModel != pNICInfo->iCardModel )
            {
               pCIS->listRadioInterfaces[i].cardModel = pNICInfo->iCardModel;
               bChanged = true;
            }
        }
        if ( bChanged )
          save_ControllerInterfacesSettings();
     }
     log_line("Finished executing start up sequence step: %d", s_StartSequence);
     s_StartSequence = START_SEQ_PRE_SYNC_DATA;
     return;
   }

   if ( s_StartSequence == START_SEQ_PRE_SYNC_DATA )
   {
      Preferences* pP = get_Preferences();
      if ( 1 == pP->iAutoExportSettings )
      {
         popupStartup.addLine("Synchronizing data...");
         s_StartSequence = START_SEQ_SYNC_DATA;
         return;
      }

      s_StartSequence = START_SEQ_PRE_LOAD_DATA;
      return;
   }

   if ( s_StartSequence == START_SEQ_SYNC_DATA )
   {
      log_line("Auto export settings is enabled. Checking boot count...");

      if ( g_iBootCount < 3 )
      {
         log_line("First or second boot (%d), skipping auto export", g_iBootCount);
         popupStartup.addLine("Synchronizing data skipped.");
         log_line("Finished executing start up sequence step: %d", s_StartSequence);
         s_StartSequence = START_SEQ_PRE_LOAD_DATA;
         return;
      }

      log_line("Doing auto export...");
      int nResult = controller_utils_export_all_to_usb();
      if ( nResult != 0 )
      {
            if ( nResult == -2 )
            {
               log_line("Failed to synchonize data to USB memory stick, no memory stick.");
               popupStartup.addLine("Synchronizing data skipped. No USB memory stick.");
            }
            else if ( nResult == -10 || nResult == -11 )
            {
               log_softerror_and_alarm("Failed to synchonize data to USB memory stick, failed to write data to USB stick.");
               popupStartup.addLine("Synchronizing data failed to write to USB memory stick.");
            }
            else
            {
               log_softerror_and_alarm("Failed to synchonize data to USB memory stick.");
               popupStartup.addLine("Synchronizing data failed.");
            }
            s_StartSequence = START_SEQ_SYNC_DATA_FAILED;
            log_line("Finished executing start up sequence step: %d", s_StartSequence);
            return;
      }
      log_line("Auto export done.");

      log_line("Finished executing start up sequence step: %d", s_StartSequence);
      popupStartup.addLine("Synchronizing data complete.");
      s_StartSequence = START_SEQ_PRE_LOAD_DATA;
      return;
   }

   if ( s_StartSequence == START_SEQ_SYNC_DATA_FAILED )
   {
      static int s_iSyncFailedLoop = 0;
      s_iSyncFailedLoop++;
      if ( s_iSyncFailedLoop > 5 )
      {
         log_line("Finished executing start up sequence step: %d", s_StartSequence);
         s_StartSequence = START_SEQ_PRE_LOAD_DATA;
      }
      return;
   }

   if ( s_StartSequence == START_SEQ_PRE_LOAD_DATA )
   {
      log_line("Start sequence: PRE_LOAD_DATA");
      popupStartup.addLine("Loading models...");
      log_line("Finished executing start up sequence step: %d", s_StartSequence);
      s_StartSequence = START_SEQ_LOAD_DATA;
      return;
   }

   if ( s_StartSequence == START_SEQ_LOAD_DATA )
   {
      log_line("Start sequence: LOAD_DATA");

      load_favorites();
      // Check file system for write access

      log_line("Checking the file system for write access...");
      bool bWriteFailed = false;

      if ( check_write_filesystem() < 0 )
      {
         alarms_add_from_local(ALARM_ID_CONTROLLER_STORAGE_WRITE_ERRROR, 0, 0);
         bWriteFailed = true;
      }
               
      if ( bWriteFailed )
         log_line("Checking the file system for write access: Failed.");
      else
         log_line("Checking the file system for write access: Succeeded.");
         
      ruby_load_models();
      
      if ( (NULL == g_pCurrentModel) || (0 == g_uActiveControllerModelVID) ||
           (g_bFirstModelPairingDone && (0 == getControllerModelsCount()) && (0 == getControllerModelsSpectatorCount())) )
      {
         if ( 0 == getControllerModelsCount() )
         {
            popupNoModel.setTitle("Info No Vehicles");
            popupNoModel.addLine("You have no vehicles linked to this controller.");
            popupNoModel.addLine("Press [Menu] key and then select 'Search' to search for a vehicle to connect to.");
            popupNoModel.setIconId(g_idIconInfo,get_Color_IconWarning());
         }
         else
         {
            popupNoModel.setTitle("Info No Active Vehicle");
            popupNoModel.addLine("You have no vehicle selected as active.");
            popupNoModel.addLine("Press [Menu] key and then select 'My Vehicles' to select the vehicle to connect to.");
            popupNoModel.setIconId(g_idIconInfo,get_Color_IconWarning());
         }

         popups_add(&popupNoModel);
         //launch_configure_nics(false, NULL);
         log_line("Finished executing start up sequence step: %d", s_StartSequence);
         s_StartSequence = START_SEQ_SCAN_MEDIA_PRE;
         return;
      }
      if ( ! load_temp_local_stats() )
         local_stats_reset_all();

      log_line("Configuring radio...");
      popupStartup.addLine("Configuring radio...");
      log_line("Finished executing start up sequence step: %d", s_StartSequence);
      s_StartSequence = START_SEQ_SCAN_MEDIA_PRE;
      return;
   }


   if ( s_StartSequence == START_SEQ_SCAN_MEDIA_PRE )
   {
      log_line("Start sequence: START_SEQ_SCAN_MEDIA_PRE");
      popupStartup.addLine("Scanning media storage...");
      log_line("Start sequence: Scanning media storage...");
      char szFile[128];
      char szFile2[128];
      strcpy(szFile, FOLDER_RUBY_TEMP);
      strcat(szFile, FILE_TEMP_VIDEO_FILE);
      strcpy(szFile2, FOLDER_RUBY_TEMP);
      strcat(szFile2, FILE_TEMP_VIDEO_FILE_INFO);
      if ( access(szFile, R_OK) != -1 )
      if ( access(szFile2, R_OK) != -1 )
      {
         long fSize = 0;
         FILE *pF = fopen(szFile, "rb");
         if ( NULL != pF )
         {
            fseek(pF, 0, SEEK_END);
            fSize = ftell(pF);
            fseek(pF, 0, SEEK_SET);
            fclose(pF);
         }

         log_line("Processing unfinished video file %s, length: %d bytes", szFile, fSize);

         char szBuff[64];
         sprintf(szBuff, "nice -n 5 ./ruby_video_proc &");
         hw_execute_bash_command(szBuff, NULL);
         log_line("Start sequence: Processing unfinished video file...");
         popupStartup.addLine("Processing unfinished video file...");
         hardware_sleep_ms(200);

         g_bVideoRecordingStarted = false;
         g_bVideoProcessing = true;
         link_watch_mark_started_video_processing();
         warnings_add(0, "Processing video file...", g_idIconCamera, get_Color_IconNormal());

         log_line("Finished executing start up sequence step: %d", s_StartSequence);
         s_StartSequence = START_SEQ_PROCESS_VIDEO;
         return;
      }
      log_line("Finished executing start up sequence step: %d", s_StartSequence);
      s_StartSequence = START_SEQ_SCAN_MEDIA;
      return;
   }
   if ( s_StartSequence == START_SEQ_PROCESS_VIDEO )
   {
      hardware_sleep_ms(200);
      /*
      char szPids[1024];
      bool procRunning = false;
      hw_execute_bash_command_silent("pidof ruby_video_proc", szPids);
      if ( strlen(szPids) > 2 )
         procRunning = true;
      if ( procRunning )
         return;

      log_line("Start sequence: Done processing temporary video file.");
      popupStartup.addLine("Done processing temporary video file.");
      */
      log_line("Start sequence: Processing temporary video file will be done in background.");
      popupStartup.addLine("Moved processing of temporary video file to background.");
      log_line("Finished executing start up sequence step: %d", s_StartSequence);
      s_StartSequence = START_SEQ_SCAN_MEDIA;
   }

   if ( s_StartSequence == START_SEQ_SCAN_MEDIA )
   {
      log_line("Start sequence: START_SEQ_SCAN_MEDIA");
      media_init_and_scan();
      Preferences* p = get_Preferences();
      popup_log_set_show_flag(p->iShowLogWindow);

      if ( is_first_boot() )
         popups_add_topmost(new Popup("One time automatic setup after instalation done: Your CPU settings have been adjusted to match your Raspberry Pi. Please reboot the controller at your convenience for the new changes to take effect.", 0.2,0.1, 0.74, 11));

      log_line("Configuring processes...");
      popupStartup.addLine("Configuring processes...");
      log_line("Finished executing start up sequence step: %d", s_StartSequence);
      s_StartSequence = START_SEQ_START_PROCESSES;
      return;
   }
   
   if ( s_StartSequence == START_SEQ_START_PROCESSES )
   {
      log_line("Start sequence: START_SEQ_START_PROCESSES");
      
      link_watch_init();
      osd_plugins_load();
      osd_widgets_load();
      r_check_processes_filesystem();

      log_line("Current local model: %X, VID: %u, first pairing done: %d, controller models: %d, spectator models: %d",
         g_pCurrentModel, g_pCurrentModel->uVehicleId, (int)g_bFirstModelPairingDone, getControllerModelsCount(), getControllerModelsSpectatorCount());
      log_line("Current active vehicle id on controller: %u", g_uActiveControllerModelVID);

      if ( (NULL == g_pCurrentModel) ||
           (g_bFirstModelPairingDone && (0 == getControllerModelsCount()) && (0 == getControllerModelsSpectatorCount())) ||
           (g_bFirstModelPairingDone && (0 == g_uActiveControllerModelVID) ) )
      {
         s_StartSequence = START_SEQ_COMPLETED;
         log_line("Start sequence: COMPLETED.");
         log_line("System Configured. Start sequence done.");
         popupStartup.addLine("System Configured.");
         popupStartup.addLine("Start sequence done.");
         popupStartup.setTimeout(4);
         popupStartup.resetTimer();
         s_TimeCentralInitializationComplete = g_TimeNow;
         _on_start_completed();
         return;
      }
      onMainVehicleChanged(true);

      if ( 0 < hardware_get_radio_interfaces_count() )
         pairing_start_normal();

      g_pProcessStatsTelemetry = shared_mem_process_stats_open_read(SHARED_MEM_WATCHDOG_TELEMETRY_RX);
      if ( NULL == g_pProcessStatsTelemetry )
         log_line("Failed to open shared mem to telemetry rx process watchdog stats for reading: %s on start. Will try later.", SHARED_MEM_WATCHDOG_TELEMETRY_RX);
      else
         log_line("Opened shared mem to telemetry rx process watchdog stats for reading.");

      g_pProcessStatsRouter = shared_mem_process_stats_open_read(SHARED_MEM_WATCHDOG_ROUTER_RX);
      if ( NULL == g_pProcessStatsRouter )
         log_line("Failed to open shared mem to video rx process watchdog stats for reading: %s on start. Will try later.", SHARED_MEM_WATCHDOG_ROUTER_RX);
      else
         log_line("Opened shared mem to video rx process watchdog stats for reading.");

      if ( NULL != g_pCurrentModel && g_pCurrentModel->rc_params.rc_enabled )
      {
         g_pProcessStatsRC = shared_mem_process_stats_open_read(SHARED_MEM_WATCHDOG_RC_TX);
         if ( NULL == g_pProcessStatsRC )
            log_line("Failed to open shared mem to RC tx process watchdog stats for reading: %s on start. Will try later.", SHARED_MEM_WATCHDOG_RC_TX);
         else
            log_line("Opened shared mem to RC tx process watchdog stats for reading.");
      }

      s_StartSequence = START_SEQ_COMPLETED;
      log_line("Start sequence: COMPLETED.");
      log_line("System Configured. Start sequence done.");
      popupStartup.addLine("System Configured.");
      if ( g_bIsReinit )
         popupStartup.addLine("Restarting checks finished. Will restart now...");
      else
         popupStartup.addLine("Start sequence done.");
      popupStartup.setTimeout(4);
      popupStartup.resetTimer();

      s_TimeCentralInitializationComplete = g_TimeNow;
      _on_start_completed();
      if ( g_bIsHDMIConfirmation )
      {
         s_pMenuConfirmHDMI = new MenuConfirmationHDMI("HDMI Output Configuration Updated","Does the HDMI output looks ok? Select [Yes] to keep the canges or select [No] to revert to the old HDMI configuration.", 0);
         s_pMenuConfirmHDMI->m_yPos = 0.3;
         add_menu_to_stack(s_pMenuConfirmHDMI);
      }

      int iMajor = 0;
      int iMinor = 0;
      get_Ruby_BaseVersion(&iMajor, &iMinor);
      if ( (iMajor < 10) || ((iMajor == 10) && (iMinor < 1)) )
      {
         MenuConfirmation* pMC = new MenuConfirmation(getString(0), getString(1), 0, true);
         pMC->addTopLine(getString(3));
         pMC->setIconId(g_idIconWarning);
         pMC->m_yPos = 0.3;
         add_menu_to_stack(pMC);
      }

      if ( g_iBootCount == 2 )
      {
         log_line("First boot detected of the UI after install. Checking for import settings from USB...");
         if ( controller_utils_usb_import_has_any_controller_id_file() )
         {
            log_line("USB has exported settings. Add confirmation to import them.");
            menu_discard_all();
            s_pMenuConfirmationImport = new MenuConfirmationImport("Automatic Import", "There are controller settings saved and present on the USB stick. Do you want to import them?", 55);
            s_pMenuConfirmationImport->m_yPos = 0.3;
            add_menu_to_stack(s_pMenuConfirmationImport);
         }
      }
      ruby_resume_watchdog();

      log_line("Finished executing start up sequence step: %d", s_StartSequence);
      return;
   }
}


void clear_shared_mems()
{
   memset(&g_SM_VideoFramesStatsOutput, 0, sizeof(shared_mem_video_frames_stats));
   //memset(&g_SM_VideoInfoStatsRadioIn, 0, sizeof(shared_mem_video_frames_stats));
   //memset(&g_VideoInfoStatsFromVehicleCameraOut, 0, sizeof(shared_mem_video_frames_stats));
   //memset(&g_VideoInfoStatsFromVehicleRadioOut, 0, sizeof(shared_mem_video_frames_stats));
   memset(&g_SM_HistoryRxStats, 0, sizeof(shared_mem_radio_stats_rx_hist));
   memset(&g_SM_HistoryRxStatsVehicle, 0, sizeof(shared_mem_radio_stats_rx_hist));
   memset(&g_SM_VideoDecodeStats, 0, sizeof(shared_mem_video_stream_stats_rx_processors));
   
   memset(&g_SMControllerRTInfo, 0, sizeof(controller_runtime_info));
   memset(&g_SMVehicleRTInfo, 0, sizeof(vehicle_runtime_info));
   memset(&g_SM_DownstreamInfoRC, 0, sizeof(t_packet_header_rc_info_downstream));
   memset(&g_SM_RouterVehiclesRuntimeInfo, 0, sizeof(shared_mem_router_vehicles_runtime_info));
   memset(&g_SM_RadioStats, 0, sizeof(shared_mem_radio_stats));
   memset(&g_SM_RadioRxQueueInfo, 0, sizeof(shared_mem_radio_rx_queue_info));
   memset(&g_SM_VideoLinkGraphs, 0, sizeof(shared_mem_video_link_graphs));
   memset(&g_SM_DevVideoBitrateHistory, 0, sizeof(shared_mem_dev_video_bitrate_history));
   memset(&g_SM_RCIn, 0, sizeof(t_shared_mem_i2c_controller_rc_in));
   memset(&g_SMVoltage, 0, sizeof(t_shared_mem_i2c_current));
}

void synchronize_shared_mems()
{
   if ( ! pairing_isStarted() )
      return;

   static u32 s_uTimeLastSyncSharedMems = 0;

   if ( g_TimeNow < s_uTimeLastSyncSharedMems + 100 )
      return;

   s_uTimeLastSyncSharedMems = g_TimeNow;

   ControllerSettings* pCS = get_ControllerSettings();

   if ( (NULL != g_pCurrentModel) && (!g_bSearching) )
   {
      if ( g_pCurrentModel->rc_params.rc_enabled )
      if ( NULL == g_pProcessStatsRC )
      {
         g_pProcessStatsRC = shared_mem_process_stats_open_read(SHARED_MEM_WATCHDOG_RC_TX);
         if ( NULL == g_pProcessStatsRC )
            log_softerror_and_alarm("Failed to open shared mem to RC tx process watchdog stats for reading: %s", SHARED_MEM_WATCHDOG_RC_TX);
         else
            log_line("Opened shared mem to RC tx process watchdog stats for reading.");
      }

      if ( NULL == g_pProcessStatsTelemetry )
      {
         g_pProcessStatsTelemetry = shared_mem_process_stats_open_read(SHARED_MEM_WATCHDOG_TELEMETRY_RX);
         if ( NULL == g_pProcessStatsTelemetry )
            log_softerror_and_alarm("Failed to open shared mem to telemetry rx process watchdog stats for reading: %s", SHARED_MEM_WATCHDOG_TELEMETRY_RX);
         else
            log_line("Opened shared mem to telemetry rx process watchdog stats for reading.");
      }

      if ( NULL == g_pProcessStatsRouter )
      {
         g_pProcessStatsRouter = shared_mem_process_stats_open_read(SHARED_MEM_WATCHDOG_ROUTER_RX);
         if ( NULL == g_pProcessStatsRouter )
            log_softerror_and_alarm("Failed to open shared mem to video rx process watchdog stats for reading: %s", SHARED_MEM_WATCHDOG_ROUTER_RX);
         else
            log_line("Opened shared mem to video rx process watchdog stats for reading.");
      }

      if ( NULL == g_pSM_HistoryRxStats )
      {
         g_pSM_HistoryRxStats = shared_mem_radio_stats_rx_hist_open_for_read();
         if ( NULL == g_pSM_HistoryRxStats )
            log_softerror_and_alarm("Failed to open history radio rx stats shared memory for read.");
      }
   }

   if ( NULL == g_pSMControllerRTInfo )
   {
      g_pSMControllerRTInfo = controller_rt_info_open_for_read();
      if ( NULL == g_pSMControllerRTInfo )
         log_softerror_and_alarm("Failed to open shared mem to controller runtime info for reading: %s", SHARED_MEM_CONTROLLER_RUNTIME_INFO);
      else
         log_line("Opened shared mem to controller runtime info for reading.");
   }
   if ( NULL != g_pSMControllerRTInfo )
   {
      memcpy((u8*)&g_SMControllerRTInfo, g_pSMControllerRTInfo, sizeof(controller_runtime_info));
      if ( (g_SMControllerRTInfo.iCurrentIndex != g_SMControllerRTInfo.iCurrentIndex2) ||
           (g_SMControllerRTInfo.iCurrentIndex2 != g_SMControllerRTInfo.iCurrentIndex3) )
      {
         if ( g_SMControllerRTInfo.iCurrentIndex2 == g_SMControllerRTInfo.iCurrentIndex3 )
            g_SMControllerRTInfo.iCurrentIndex = g_SMControllerRTInfo.iCurrentIndex2;
      }
   }
   if ( NULL == g_pSMVehicleRTInfo )
   {
      g_pSMVehicleRTInfo = vehicle_rt_info_open_for_read();
      if ( NULL == g_pSMVehicleRTInfo )
         log_softerror_and_alarm("Failed to open shared mem to vehicle runtime info for reading: %s", SHARED_MEM_VEHICLE_RUNTIME_INFO);
      else
         log_line("Opened shared mem to vehicle runtime info for reading.");
   }
   if ( NULL != g_pSMVehicleRTInfo )
      memcpy((u8*)&g_SMVehicleRTInfo, g_pSMVehicleRTInfo, sizeof(vehicle_runtime_info));


   if ( g_bFreezeOSD )
      return;

   if ( NULL != g_pProcessStatsRouter )
      memcpy((u8*)&g_ProcessStatsRouter, g_pProcessStatsRouter, sizeof(shared_mem_process_stats));
   if ( NULL != g_pProcessStatsTelemetry )
      memcpy((u8*)&g_ProcessStatsTelemetry, g_pProcessStatsTelemetry, sizeof(shared_mem_process_stats));
   if ( NULL != g_pProcessStatsRC )
      memcpy((u8*)&g_ProcessStatsRC, g_pProcessStatsRC, sizeof(shared_mem_process_stats));

   if ( NULL != g_pSM_DownstreamInfoRC )
      memcpy((u8*)&g_SM_DownstreamInfoRC, g_pSM_DownstreamInfoRC, sizeof(t_packet_header_rc_info_downstream));

   if ( NULL != g_pSM_RouterVehiclesRuntimeInfo )
      memcpy((u8*)&g_SM_RouterVehiclesRuntimeInfo, g_pSM_RouterVehiclesRuntimeInfo, sizeof(shared_mem_router_vehicles_runtime_info));
   if ( NULL != g_pSM_RadioStats )
      memcpy((u8*)&g_SM_RadioStats, g_pSM_RadioStats, sizeof(shared_mem_radio_stats));
   
   if ( NULL != g_pSM_HistoryRxStats )
      memcpy((u8*)&g_SM_HistoryRxStats, g_pSM_HistoryRxStats, sizeof(shared_mem_radio_stats_rx_hist));
   
   if ( pCS->iDeveloperMode )
   if ( NULL != g_pCurrentModel )
   if ( g_pCurrentModel->osd_params.osd_flags[g_pCurrentModel->osd_params.iCurrentOSDLayout] & OSD_FLAG_SHOW_STATS_VIDEO_H264_FRAMES_INFO)
   {
      if ( NULL != g_pSM_VideoFramesStatsOutput )
      if ( g_TimeNow >= g_SM_VideoFramesStatsOutput.uLastTimeStatsUpdate + 200 )
         memcpy((u8*)&g_SM_VideoFramesStatsOutput, g_pSM_VideoFramesStatsOutput, sizeof(shared_mem_video_frames_stats));
      //if ( NULL != g_pSM_VideoInfoStatsRadioIn )
      //if ( g_TimeNow >= g_SM_VideoInfoStatsRadioIn.uLastTimeStatsUpdate + 200 )
      //   memcpy((u8*)&g_SM_VideoInfoStatsRadioIn, g_pSM_VideoInfoStatsRadioIn, sizeof(shared_mem_video_frames_stats));
   }

   if ( NULL != g_pSM_VideoDecodeStats )
      memcpy((u8*)&g_SM_VideoDecodeStats, g_pSM_VideoDecodeStats, sizeof(shared_mem_video_stream_stats_rx_processors));
   if ( NULL != g_pSM_RadioRxQueueInfo )
      memcpy((u8*)&g_SM_RadioRxQueueInfo, g_pSM_RadioRxQueueInfo, sizeof(shared_mem_radio_rx_queue_info));
   // To fix
   //if ( NULL != g_pSM_VideoLinkStats )
   //   memcpy((u8*)&g_SM_VideoLinkStats, g_pSM_VideoLinkStats, sizeof(shared_mem_video_link_stats_and_overwrites));
   if ( NULL != g_pSM_VideoLinkGraphs )
      memcpy((u8*)&g_SM_VideoLinkGraphs, g_pSM_VideoLinkGraphs, sizeof(shared_mem_video_link_graphs));
   if ( NULL != g_pSM_RCIn )
      memcpy((u8*)&g_SM_RCIn, g_pSM_RCIn, sizeof(t_shared_mem_i2c_controller_rc_in));
   if ( NULL != g_pSMVoltage )
      memcpy((u8*)&g_SMVoltage, g_pSMVoltage, sizeof(t_shared_mem_i2c_current));

}

void ruby_processing_loop(bool bNoKeys)
{
   ControllerSettings* pCS = get_ControllerSettings();

   hardware_sleep_ms(10);

   u32 uTimeStart = get_current_timestamp_ms();

   try_read_messages_from_router(5);

   keyboard_consume_input_events();
   u32 uSumEvent = keyboard_get_triggered_input_events();

   if ( uSumEvent & 0xFF0000 )
      warnings_add_input_device_unknown_key((int)((uSumEvent >> 16) & 0xFF));
   handle_commands_loop();

   pairing_loop();

   synchronize_shared_mems();

    if ( pCS->iFreezeOSD )
    if ( pCS->iDeveloperMode )
    if ( keyboard_get_triggered_input_events() & INPUT_EVENT_PRESS_BACK )
    if ( ! isMenuOn() )
    if ( g_TimeNow > s_uTimeFreezeOSD + 200 )
    {
       s_uTimeFreezeOSD = g_TimeNow;
       s_bFreezeOSD = ! s_bFreezeOSD;
    }

    s_TimeLastMenuInput = g_TimeNow;
    if ( ! bNoKeys )
    {
       menu_loop();
       //if ( keyboard_has_long_press_flag() )
       //   menu_loop_parse_input_events();
       if ( ! exectuteActionsInputDebugStats() )
       if ( keyboard_get_triggered_input_events() & (INPUT_EVENT_PRESS_QA1 | INPUT_EVENT_PRESS_QA2 | INPUT_EVENT_PRESS_QA3) )
          executeQuickActions();
    }

   if ( g_iMustSendCurrentActiveOSDLayoutCounter > 0 )
   if ( g_TimeNow >= g_TimeLastSentCurrentActiveOSDLayout+200 )
   if ( (NULL != g_pCurrentModel) && link_is_vehicle_online_now(g_pCurrentModel->uVehicleId)  )
   {
      g_iMustSendCurrentActiveOSDLayoutCounter--;
      g_TimeLastSentCurrentActiveOSDLayout = g_TimeNow;
      handle_commands_decrement_command_counter();
      handle_commands_send_single_oneway_command(0, COMMAND_ID_SET_OSD_CURRENT_LAYOUT, (u32)g_pCurrentModel->osd_params.iCurrentOSDLayout, NULL, 0);
   }

   if ( g_bIsRouterReady )
   {
      local_stats_update_loop();
      forward_streams_loop();
      link_watch_loop();
      warnings_periodic_loop();
   }

   u32 uTime10 = get_current_timestamp_ms();
   u32 dTime = uTime10 - uTimeStart;
   if ( dTime > 200 )
   if ( (s_StartSequence == START_SEQ_COMPLETED) || (s_StartSequence == START_SEQ_FAILED) )
      log_softerror_and_alarm("Main processing loop took too long (%u ms)", dTime);
}

void main_loop_r_central()
{
   ControllerSettings* pCS = get_ControllerSettings();

   hardware_sleep_ms(2);

   ruby_processing_loop(false);

   if ( s_StartSequence != START_SEQ_COMPLETED && s_StartSequence != START_SEQ_FAILED )
   {
      hardware_sleep_ms(5);
      start_loop();
      log_line("Startup sequence now after executing a step: %d", s_StartSequence);
      render_all(g_TimeNow, false, false);
      if ( NULL != g_pProcessStatsCentral )
         g_pProcessStatsCentral->lastActiveTime = g_TimeNow;
      log_line("Startup sequence now after rendering a step: %d", s_StartSequence);
      return;
   }

   if ( s_StartSequence != START_SEQ_COMPLETED )
      return;

   if ( g_bMarkedHDMIReinit )
   {
      g_bMarkedHDMIReinit = false;
      menu_discard_all();
      ruby_reinit_hdmi_display();
   }

   compute_cpu_state();

   int dt = 1000/15;
   if ( 0 != pCS->iRenderFPS )
      dt = 1000/pCS->iRenderFPS;
   if ( g_TimeNow >= s_TimeLastRender+dt )
   {
      ruby_signal_alive();
      s_TimeLastRender = g_TimeNow;
      render_all(g_TimeNow, false, false);
      if ( NULL != g_pProcessStatsCentral )
         g_pProcessStatsCentral->lastActiveTime = g_TimeNow;

      if ( g_bIsReinit )
      if ( s_iFPSCount > 5 )
         g_bQuit = true;
   }

   if ( g_bIsHDMIConfirmation )
   if ( NULL != s_pMenuConfirmHDMI )
   if ( g_TimeNow > s_TimeCentralInitializationComplete + 10000 )
   if ( menu_is_menu_on_top(s_pMenuConfirmHDMI) )
   if ( access(s_szFileHDMIChanged, R_OK) != -1 )         
   {
      log_line("Reverting HDMI resolution change...");
      ruby_pause_watchdog();
      save_temp_local_stats();
      hardware_sleep_ms(50);

      FILE* fd = fopen(s_szFileHDMIChanged, "r");
      if ( NULL != fd )
      {
         char szBuff[256];
         int group, mode;
         int tmp1, tmp2, tmp3;
         fscanf(fd, "%d %d", &tmp1, &tmp2 );
         fscanf(fd, "%d %d %d", &tmp1, &tmp2, &tmp3 );
         fscanf(fd, "%d %d", &group, &mode );
         fclose(fd);
         log_line("Reverting HDMI resolution back to: group: %d, mode: %d", group, mode);

         sprintf(szBuff, "rm -rf %s%s", FOLDER_CONFIG, FILE_TEMP_HDMI_CHANGED);
         hw_execute_bash_command(szBuff, NULL);

         hw_execute_bash_command("cp /boot/config.txt config.txt", NULL);

         sprintf(szBuff, "sed -i 's/hdmi_group=[0-9]*/hdmi_group=%d/g' config.txt", group);
         hw_execute_bash_command(szBuff, NULL);
         sprintf(szBuff, "sed -i 's/hdmi_mode=[0-9]*/hdmi_mode=%d/g' config.txt", mode);
         hw_execute_bash_command(szBuff, NULL);
         hw_execute_bash_command("cp config.txt /boot/config.txt", NULL);
      }
      onEventReboot();
      hardware_reboot();
   }
}

void ruby_signal_alive()
{
   if ( NULL != g_pProcessStatsCentral )
      g_pProcessStatsCentral->lastActiveTime = g_TimeNow;
   else
      log_softerror_and_alarm("Shared mem for self process stats is NULL. Can't signal to others that process is active.");
}

void ruby_pause_watchdog()
{
   g_TimeNow = get_current_timestamp_ms();
   ruby_signal_alive();
   s_iCountRequestsPauseWatchdog++;
   if ( 1 == s_iCountRequestsPauseWatchdog )
   {
      log_line("[Watchdog] Pause watchdog [%d], signal others.", s_iCountRequestsPauseWatchdog);
      char szComm[128];
      sprintf(szComm, "touch %s%s", FOLDER_RUBY_TEMP, FILE_TEMP_CONTROLLER_PAUSE_WATCHDOG);
      hw_execute_bash_command_silent(szComm, NULL);
   }
   else
      log_line("[Watchdog] Pause watchdog [%d].", s_iCountRequestsPauseWatchdog);
}

void ruby_resume_watchdog()
{
   g_TimeNow = get_current_timestamp_ms();
   ruby_signal_alive();
   s_iCountRequestsPauseWatchdog--;
   if ( 0 == s_iCountRequestsPauseWatchdog )
   {
      hardware_sleep_ms(20);
      log_line("[Watchdog] Resumed watchdog [%d], signal others.", s_iCountRequestsPauseWatchdog);
      char szComm[128];
      sprintf(szComm, "rm -rf %s%s", FOLDER_RUBY_TEMP, FILE_TEMP_CONTROLLER_PAUSE_WATCHDOG);
      hw_execute_bash_command_silent(szComm, NULL);
   }
   else
   {
      if ( s_iCountRequestsPauseWatchdog < 0 )
         s_iCountRequestsPauseWatchdog = 0;
      log_line("[Watchdog] Resumed watchdog [%d].", s_iCountRequestsPauseWatchdog);
   }
}

   
void handle_sigint(int sig) 
{ 
   log_line("--------------------------");
   log_line("Caught signal to stop: %d", sig);
   log_line("--------------------------");
   g_bQuit = true;
} 

int main(int argc, char *argv[])
{
   signal(SIGPIPE, SIG_IGN);
   signal(SIGINT, handle_sigint);
   signal(SIGTERM, handle_sigint);
   signal(SIGQUIT, handle_sigint);

   if ( strcmp(argv[argc-1], "-ver") == 0 )
   {
      printf("%d.%d (b%d)", SYSTEM_SW_VERSION_MAJOR, SYSTEM_SW_VERSION_MINOR/10, SYSTEM_SW_BUILD_NUMBER);
      return 0;
   }

   g_bDebugState = false;
   g_bDebugStats = false;

   if ( argc >= 1 )
   if ( strcmp(argv[argc-1], "-ds") == 0 )
   {
      g_bDebugStats = true;
      log_line("Starting with debug stats...");
   }
   
   if ( argc >= 1 )
   if ( strcmp(argv[argc-1], "-debug") == 0 )
      g_bDebugState = true;

   if ( access(CONFIG_FILENAME_DEBUG, R_OK) != -1 )
      g_bDebugState = true;
   if ( g_bDebugState )
      log_line("Starting in debug mode.");

   
   setlocale(LC_ALL, "en_GB.UTF-8");

   s_idBgImage[0] = 0;
   s_idBgImage[1] = 0;
   s_idBgImage[2] = 0;
   s_idBgImageMenu[0] = 0;
   s_idBgImageMenu[1] = 0;
   s_idBgImageMenu[2] = 0;

   log_init("Central");

   check_licences();
   
   char szFile[MAX_FILE_PATH_SIZE];

   g_uControllerId = controller_utils_getControllerId();
   log_line("Controller UID: %u", g_uControllerId);
 
   if ( strcmp(argv[argc-1], "-reinit") == 0 )
   {
      log_line("Ruby Central crashed last time, reinitializing graphics engine...");
      g_bIsReinit = true;
   }

   strcpy(s_szFileHDMIChanged, FOLDER_CONFIG);
   strcat(s_szFileHDMIChanged, FILE_TEMP_HDMI_CHANGED);

   g_bIsHDMIConfirmation = false;
   if ( access(s_szFileHDMIChanged, R_OK) != -1 )
      g_bIsHDMIConfirmation = true;

   log_line("Ruby UI starting");

   init_hardware();

   if ( ! load_Preferences() )
      save_Preferences();

   if ( ! load_ControllerSettings() )
      save_ControllerSettings();

   Preferences* p = get_Preferences();
   if ( p->nLogLevel != 0 )
      log_only_errors();

   ControllerSettings* pCS = get_ControllerSettings();
   
   #if defined (HW_PLATFORM_RASPBERRY)
   hdmi_enum_modes();
   #endif

   #if defined (HW_PLATFORM_RADXA_ZERO3)
   ruby_drm_core_wait_for_display_connected();
   hdmi_enum_modes();
   int iHDMIIndex = hdmi_load_current_mode();
   if ( iHDMIIndex < 0 )
      iHDMIIndex = hdmi_get_best_resolution_index_for(DEFAULT_RADXA_DISPLAY_WIDTH, DEFAULT_RADXA_DISPLAY_HEIGHT, DEFAULT_RADXA_DISPLAY_REFRESH);
   log_line("HDMI mode to use: %d (%d x %d @ %d)", iHDMIIndex, hdmi_get_current_resolution_width(), hdmi_get_current_resolution_height(), hdmi_get_current_resolution_refresh() );
   ruby_drm_core_init(0, DRM_FORMAT_ARGB8888, hdmi_get_current_resolution_width(), hdmi_get_current_resolution_height(), hdmi_get_current_resolution_refresh());
   ruby_drm_core_set_plane_properties_and_buffer(ruby_drm_core_get_main_draw_buffer_id());
   #endif

   g_pRenderEngine = render_init_engine();
   log_line("Render Engine was initialized.");

   if ( g_bPlayIntro )
   {
      if ( g_bIsReinit )
         g_bPlayIntro = false;
      else
      {
         #ifdef HW_PLATFORM_RASPBERRY
         char szComm[256];
         strcpy(szFile, FOLDER_RUBY_TEMP);
         strcat(szFile, FILE_TEMP_INTRO_PLAYING);
         sprintf(szComm, "touch %s", szFile);
         hw_execute_bash_command(szComm, NULL);
         sprintf(szComm, "./%s res/intro.h264 15 -endexit&", VIDEO_PLAYER_OFFLINE);
         hw_execute_bash_command_nonblock(szComm, NULL);
         #endif
         #ifdef HW_PLATFORM_RADXA_ZERO3
         //for( int i=0; i<25; i++ )
         //   hardware_sleep_ms(100);
         //sprintf(szComm, "./%s -f res/intro.h264 20 -endexit&", VIDEO_PLAYER_OFFLINE);
         //hw_execute_bash_command_nonblock(szComm, NULL);
         #endif

         hardware_enable_audio_output();
         hardware_set_audio_output_volume(pCS->iAudioOutputVolume);

         //int iSample = 1 + (rand()%2);
         //char szFileIntro[256];
         //sprintf(szFileIntro, "res/intro%d.wav", iSample);
         hardware_audio_play_file_async("res/intro1.wav");

      }
   }

   load_resources();
   osd_apply_preferences();
   menu_init();
   Menu::setRenderMode(p->iMenuStyle);
   hardware_swap_buttons(p->iSwapUpDownButtons);
   warnings_remove_all();

   hardware_init_serial_ports();

   clear_shared_mems();   
   ruby_clear_all_ipc_channels();

   g_pProcessStatsCentral = shared_mem_process_stats_open_write(SHARED_MEM_WATCHDOG_CENTRAL);
   if ( NULL == g_pProcessStatsCentral )
      log_softerror_and_alarm("Failed to open shared mem for ruby_central process watchdog for writing: %s", SHARED_MEM_WATCHDOG_CENTRAL);
   else
      log_line("Opened shared mem for ruby_centrall process watchdog for writing.");
 
   ruby_pause_watchdog();

   controller_compute_cpu_info();

   hardware_i2c_load_device_settings();

   if ( ! load_ControllerInterfacesSettings() )
      save_ControllerInterfacesSettings();
      
   save_ControllerInterfacesSettings();

   #if defined (HW_PLATFORM_RADXA_ZERO3)
   log_line("Ruby OLED Init...");
   if ( hardware_i2c_has_oled_screen() )
   {
      oled_render_init();
      oled_render_thread_start();
   }
   #endif

   if ( pCS->iPrioritiesAdjustment )
      hw_set_priority_current_proc(pCS->iNiceCentral); 


   g_TimeNow = get_current_timestamp_ms();
   s_uTimeLastChangeBgImage = g_TimeNow;

   log_line("Started.");

   popupStartup.setFont(g_idFontMenu);
   if ( g_bPlayIntro )
   {
      popupStartup.setBackgroundAlpha(0.3);
      popupStartup.setXPos(0.03);
      popupStartup.setYPos(0.05);
   }
   popupNoModel.setFont(g_idFontMenu);
   popups_add(&popupStartup);

   alarms_reset();

   if ( g_bIsReinit )
   {
      Popup* p = new Popup(true, "User Interface is reinitializing, please wait...", 0);
      p->setFont(g_idFontOSDBig);
      p->setIconId(g_idIconInfo, get_Color_IconWarning());
      popups_add(p);
   }

   g_iBootCount = 0;

   strcpy(szFile, FOLDER_CONFIG);
   strcat(szFile, FILE_CONFIG_BOOT_COUNT);
   FILE* fd = fopen(szFile, "r");
   if ( NULL != fd )
   {
      fscanf(fd, "%d", &g_iBootCount);
      fclose(fd);
      fd = NULL;
   }

   shared_vars_state_reset_all_vehicles_runtime_info();

   g_bFirstModelPairingDone = false;

   strcpy(szFile, FOLDER_CONFIG);
   strcat(szFile, FILE_CONFIG_FIRST_PAIRING_DONE); 
   if ( access(szFile, R_OK) != -1 )
      g_bFirstModelPairingDone = true;
 
   keyboard_init();

   controller_rt_info_init(&g_SMControllerRTInfo);
   vehicle_rt_info_init(&g_SMVehicleRTInfo);

   s_StartSequence = START_SEQ_PRE_LOAD_CONFIG;
   log_line("Started main loop.");
   g_TimeStart = get_current_timestamp_ms();

   log_line("Start main loop.");

   while (!g_bQuit) 
   {
      g_TimeNow = get_current_timestamp_ms();
      g_TimeNowMicros = get_current_timestamp_micros();

      if ( rx_scope_is_started() )
      {
         try_read_messages_from_router(10);
         rx_scope_loop();
      }
      else
      {
         main_loop_r_central();
      }
   }
   
   keyboard_uninit();
   
   if ( ! g_bIsReinit )
      pairing_stop();

   #if defined (HW_PLATFORM_RADXA_ZERO3)
   oled_render_shutdown();
   #endif
   
   controller_stop_i2c();
   log_line("Central: Releasing %d OSD plugins...", g_iPluginsOSDCount);
   for( int i=0; i<g_iPluginsOSDCount; i++ )
      if ( NULL != g_pPluginsOSD[i] )
      if ( NULL != g_pPluginsOSD[i]->pLibrary )
         dlclose(g_pPluginsOSD[i]->pLibrary);

   shared_mem_i2c_current_close(g_pSMVoltage);
   shared_mem_i2c_rotary_encoder_buttons_events_close(g_pSMRotaryEncoderButtonsEvents);

   shared_mem_process_stats_close(SHARED_MEM_WATCHDOG_TELEMETRY_RX, g_pProcessStatsTelemetry);
   shared_mem_process_stats_close(SHARED_MEM_WATCHDOG_ROUTER_RX, g_pProcessStatsRouter);
   shared_mem_process_stats_close(SHARED_MEM_WATCHDOG_RC_TX, g_pProcessStatsRC);

   shared_mem_process_stats_close(SHARED_MEM_WATCHDOG_CENTRAL, g_pProcessStatsCentral);
 
   hardware_release();

   render_free_engine();

   #if defined (HW_PLATFORM_RADXA_ZERO3)
   ruby_drm_core_uninit();
   #endif

   return 0;
}


void ruby_set_active_model_id(u32 uVehicleId)
{
   g_uActiveControllerModelVID = uVehicleId;
   Model* pModel = NULL;
   if ( uVehicleId != 0 )
   {
     pModel = findModelWithId(uVehicleId, 62);
     if ( NULL == pModel )
        log_line("Ruby: Set active model vehicle id to %u (no model found on controller for this VID)", g_uActiveControllerModelVID );
     else
        log_line("Ruby: Set active model vehicle id to %u, mode: %s", g_uActiveControllerModelVID, (pModel->is_spectator)?"spectator mode":"control mode");
   }
   else
      log_line("Ruby: Set active model vehicle id to 0");

   char szFile[128];
   strcpy(szFile, FOLDER_CONFIG);
   strcat(szFile, FILE_CONFIG_ACTIVE_CONTROLLER_MODEL);
   FILE* fd = fopen(szFile, "wb");
   if ( NULL != fd )
   {
      fprintf(fd, "%u\n", g_uActiveControllerModelVID);
      fclose(fd);
   }
   else
      log_softerror_and_alarm("Ruby: Failed to save active model vehicle id");
}

void ruby_mark_reinit_hdmi_display()
{
   g_bMarkedHDMIReinit = true;
}

void ruby_reinit_hdmi_display()
{
   log_line("Reinit HDMI display...");

   pairing_stop();

   free_all_fonts();
   render_free_engine();
   
   #if defined (HW_PLATFORM_RADXA_ZERO3)
   ruby_drm_core_uninit();
   ruby_drm_core_wait_for_display_connected();

   hdmi_enum_modes();
   int iHDMIIndex = hdmi_load_current_mode();
   if ( iHDMIIndex < 0 )
      iHDMIIndex = hdmi_get_best_resolution_index_for(DEFAULT_RADXA_DISPLAY_WIDTH, DEFAULT_RADXA_DISPLAY_HEIGHT, DEFAULT_RADXA_DISPLAY_REFRESH);
   log_line("HDMI mode to use: %d (%d x %d @ %d)", iHDMIIndex, hdmi_get_current_resolution_width(), hdmi_get_current_resolution_height(), hdmi_get_current_resolution_refresh() );
   ruby_drm_core_init(0, DRM_FORMAT_ARGB8888, hdmi_get_current_resolution_width(), hdmi_get_current_resolution_height(), hdmi_get_current_resolution_refresh());
   ruby_drm_core_set_plane_properties_and_buffer(ruby_drm_core_get_main_draw_buffer_id());
   #endif

   g_pRenderEngine = render_init_engine();
   log_line("Render Engine was initialized.");
   
   load_resources();
   osd_apply_preferences();
   menu_init();

   log_line("Done reinit HDMI display.");

   pairing_start_normal(); 
}