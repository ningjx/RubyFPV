/*
    Ruby Licence
    Copyright (c) 2025 Petru Soroaga
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

#include "base.h"
#include "config.h"
#include "ctrl_settings.h"
#include "hardware.h"
#include "hardware_radio.h"
#include "hw_procs.h"
#include "flags.h"

#if defined(HW_PLATFORM_RASPBERRY) || defined(HW_PLATFORM_RADXA_ZERO3)

ControllerSettings s_CtrlSettings;
int s_CtrlSettingsLoaded = 0;

void reset_ControllerSettings()
{
   memset(&s_CtrlSettings, 0, sizeof(s_CtrlSettings));
   s_CtrlSettings.iUseBrokenVideoCRC = 0;
   s_CtrlSettings.iFixedTxPower = 0;
   s_CtrlSettings.iHDMIBoost = 6;
   s_CtrlSettings.iCoresAdjustment = 1;
   s_CtrlSettings.iPrioritiesAdjustment = 1;
   s_CtrlSettings.iOverVoltage = 0;
   s_CtrlSettings.iFreqARM = 0;
   s_CtrlSettings.iFreqGPU = 0;

   s_CtrlSettings.iNiceRouter = DEFAULT_PRIORITY_PROCESS_ROUTER;
   s_CtrlSettings.ioNiceRouter = DEFAULT_IO_PRIORITY_ROUTER;
   s_CtrlSettings.iNiceCentral = DEFAULT_PRIORITY_PROCESS_CENTRAL;
   s_CtrlSettings.iNiceRXVideo = DEFAULT_PRIORITY_PROCESS_VIDEO_RX;
   s_CtrlSettings.ioNiceRXVideo = DEFAULT_IO_PRIORITY_VIDEO_RX;
   s_CtrlSettings.iVideoForwardUSBType = 0;
   s_CtrlSettings.iVideoForwardUSBPort = 5001;
   s_CtrlSettings.iVideoForwardUSBPacketSize = 1024;
   s_CtrlSettings.nVideoForwardETHType = 0;
   s_CtrlSettings.nVideoForwardETHPort = 5010;
   s_CtrlSettings.nVideoForwardETHPacketSize = 1024;
   s_CtrlSettings.iTelemetryForwardUSBType = 0;
   s_CtrlSettings.iTelemetryForwardUSBPort = 5002;
   s_CtrlSettings.iTelemetryForwardUSBPacketSize = 128;
   s_CtrlSettings.iTelemetryOutputSerialPortIndex = -1;
   s_CtrlSettings.iTelemetryInputSerialPortIndex = -1;
   s_CtrlSettings.iMAVLinkSysIdController = DEFAULT_MAVLINK_SYS_ID_CONTROLLER;

   s_CtrlSettings.iDisableHDMIOverscan = 0;
   s_CtrlSettings.iDeveloperMode = 0;
   s_CtrlSettings.iRenderFPS = 10;
   s_CtrlSettings.iShowVoltage = 1;
   s_CtrlSettings.nRetryRetransmissionAfterTimeoutMS = DEFAULT_VIDEO_RETRANS_MINIMUM_RETRY_INTERVAL;
   s_CtrlSettings.nRequestRetransmissionsOnVideoSilenceMs = DEFAULT_VIDEO_RETRANS_REQUEST_ON_VIDEO_SILENCE_MS;
   s_CtrlSettings.nUseFixedIP = 0;
   s_CtrlSettings.uFixedIP = (192<<24) | (168<<16) | (1<<8) | 20;
   s_CtrlSettings.nAutomaticTxCard = 1;
   s_CtrlSettings.nRotaryEncoderFunction = 1; // 0 - none, 1 - menu, 2 - camera
   s_CtrlSettings.nRotaryEncoderSpeed = 0; // 0 - normal, 1 - slow
   s_CtrlSettings.nRotaryEncoderFunction2 = 2; // 0 - none, 1 - menu, 2 - camera
   s_CtrlSettings.nRotaryEncoderSpeed2 = 0; // 0 - normal, 1 - slow
   s_CtrlSettings.nPingClockSyncFrequency = DEFAULT_PING_FREQUENCY;
   s_CtrlSettings.nGraphRadioRefreshInterval = 50;
   s_CtrlSettings.nGraphVideoRefreshInterval = 50;
   s_CtrlSettings.iDisableRetransmissionsAfterControllerLinkLostMiliseconds = DEFAULT_CONTROLLER_LINK_MILISECONDS_TIMEOUT_TO_DISABLE_RETRANSMISSIONS;
   s_CtrlSettings.iVideoDecodeStatsSnapshotClosesOnTimeout = 1;
   s_CtrlSettings.iFreezeOSD = 0;
   s_CtrlSettings.iDevSwitchVideoProfileUsingQAButton = -1;
   s_CtrlSettings.iShowControllerAdaptiveInfoStats = 0;
   s_CtrlSettings.iShowVideoStreamInfoCompactType = 1; // 0 full, 1 compact, 2 minimal

   s_CtrlSettings.iSearchSiKAirRate = DEFAULT_RADIO_DATARATE_SIK_AIR;
   s_CtrlSettings.iSearchSiKECC = 0;
   s_CtrlSettings.iSearchSiKLBT = 0;
   s_CtrlSettings.iSearchSiKMCSTR = 0;

   s_CtrlSettings.iAudioOutputDevice = 1;
   s_CtrlSettings.iAudioOutputVolume = 100;

   s_CtrlSettings.iDevRxLoopTimeout = DEFAULT_MAX_RX_LOOP_TIMEOUT_MILISECONDS;
   s_CtrlSettings.uShowBigRxHistoryInterface = 0;
   s_CtrlSettings.iSiKPacketSize = DEFAULT_SIK_PACKET_SIZE;

   s_CtrlSettings.iRadioRxThreadPriority = DEFAULT_PRIORITY_THREAD_RADIO_RX;
   s_CtrlSettings.iRadioTxThreadPriority = DEFAULT_PRIORITY_THREAD_RADIO_TX;
   s_CtrlSettings.iRadioTxUsesPPCAP = DEFAULT_USE_PPCAP_FOR_TX;
   s_CtrlSettings.iRadioBypassSocketBuffers = DEFAULT_BYPASS_SOCKET_BUFFERS;
   s_CtrlSettings.iStreamerOutputMode = 0;
   s_CtrlSettings.iVideoMPPBuffersSize = DEFAULT_MPP_BUFFERS_SIZE;
   if ( s_CtrlSettingsLoaded )
      log_line("Reseted controller settings.");
}

int save_ControllerSettings()
{
   char szFile[128];
   strcpy(szFile, FOLDER_CONFIG);
   strcat(szFile, FILE_CONFIG_CONTROLLER_SETTINGS);
   FILE* fd = fopen(szFile, "w");
   if ( NULL == fd )
   {
      log_softerror_and_alarm("Failed to save controller settings to file: %s", szFile);
      return 0;
   }
   fprintf(fd, "%s\n", CONTROLLER_SETTINGS_STAMP_ID);
   fprintf(fd, "%d %d %d\n", s_CtrlSettings.iDeveloperMode, s_CtrlSettings.iUseBrokenVideoCRC, s_CtrlSettings.iHDMIBoost);
   fprintf(fd, "%d %d %d\n", s_CtrlSettings.iOverVoltage, s_CtrlSettings.iFreqARM, s_CtrlSettings.iFreqGPU);

   fprintf(fd, "%d %d %d\n%d %d\n", s_CtrlSettings.iNiceRouter, s_CtrlSettings.iNiceCentral, s_CtrlSettings.iNiceRXVideo, s_CtrlSettings.ioNiceRouter, s_CtrlSettings.ioNiceRXVideo);

   fprintf(fd, "video_usb: %d %d %d\n", s_CtrlSettings.iVideoForwardUSBType, s_CtrlSettings.iVideoForwardUSBPort, s_CtrlSettings.iVideoForwardUSBPacketSize);
   fprintf(fd, "telem_usb: %d %d %d\n", s_CtrlSettings.iTelemetryForwardUSBType, s_CtrlSettings.iTelemetryForwardUSBPort, s_CtrlSettings.iTelemetryForwardUSBPacketSize);
   fprintf(fd, "%d %d\n", s_CtrlSettings.iTelemetryOutputSerialPortIndex, s_CtrlSettings.iTelemetryInputSerialPortIndex);

   fprintf(fd, "%d %d\n", s_CtrlSettings.iMAVLinkSysIdController, s_CtrlSettings.iDisableHDMIOverscan);
   fprintf(fd, "%d %d\n", s_CtrlSettings.iRenderFPS, s_CtrlSettings.iShowVoltage);

   fprintf(fd, "%d %d\n", s_CtrlSettings.nRetryRetransmissionAfterTimeoutMS, s_CtrlSettings.nRequestRetransmissionsOnVideoSilenceMs);

   fprintf(fd, "%d %u\n", s_CtrlSettings.nUseFixedIP, s_CtrlSettings.uFixedIP);

   fprintf(fd, "video_eth: %d %d %d\n", s_CtrlSettings.nVideoForwardETHType, s_CtrlSettings.nVideoForwardETHPort, s_CtrlSettings.nVideoForwardETHPacketSize);

   fprintf(fd, "%d\n", s_CtrlSettings.nAutomaticTxCard);
   fprintf(fd, "%d %d\n", s_CtrlSettings.nRotaryEncoderFunction, s_CtrlSettings.nRotaryEncoderSpeed);
   fprintf(fd, "%d %d %d\n", s_CtrlSettings.nPingClockSyncFrequency, s_CtrlSettings.nGraphRadioRefreshInterval, s_CtrlSettings.nGraphVideoRefreshInterval);

   // Extra params

   fprintf(fd, "%d %d\n", s_CtrlSettings.iDisableRetransmissionsAfterControllerLinkLostMiliseconds, s_CtrlSettings.iVideoDecodeStatsSnapshotClosesOnTimeout);
   fprintf(fd, "%d %d\n", s_CtrlSettings.nRotaryEncoderFunction2, s_CtrlSettings.nRotaryEncoderSpeed2);
   fprintf(fd, "%d %d\n", -1, s_CtrlSettings.iFreezeOSD);
   fprintf(fd, "%d %d\n", s_CtrlSettings.iDevSwitchVideoProfileUsingQAButton, s_CtrlSettings.iShowControllerAdaptiveInfoStats);
   fprintf(fd, "%d\n", s_CtrlSettings.iShowVideoStreamInfoCompactType);

   fprintf(fd, "%d %d %d %d\n", s_CtrlSettings.iSearchSiKAirRate, s_CtrlSettings.iSearchSiKECC, s_CtrlSettings.iSearchSiKLBT, s_CtrlSettings.iSearchSiKMCSTR);
   fprintf(fd, "%d %d\n", s_CtrlSettings.iAudioOutputDevice, s_CtrlSettings.iAudioOutputVolume);
   fprintf(fd, "%d %u\n", s_CtrlSettings.iDevRxLoopTimeout, s_CtrlSettings.uShowBigRxHistoryInterface);
   fprintf(fd, "%d\n", s_CtrlSettings.iSiKPacketSize);
   fprintf(fd, "%d %d\n", s_CtrlSettings.iRadioRxThreadPriority, s_CtrlSettings.iRadioTxThreadPriority);
   fprintf(fd, "%d %d %d\n", s_CtrlSettings.iRadioTxUsesPPCAP, s_CtrlSettings.iRadioBypassSocketBuffers, s_CtrlSettings.iFixedTxPower);
   fprintf(fd, "%d %d\n", s_CtrlSettings.iCoresAdjustment, s_CtrlSettings.iPrioritiesAdjustment);
   fprintf(fd, "%d %d\n", s_CtrlSettings.iStreamerOutputMode, s_CtrlSettings.iVideoMPPBuffersSize);
   fclose(fd);

   log_line("Saved controller settings to file: %s", szFile);
   return 1;
}

int load_ControllerSettings()
{
   reset_ControllerSettings();
   s_CtrlSettingsLoaded = 1;

   char szFile[128];
   strcpy(szFile, FOLDER_CONFIG);
   strcat(szFile, FILE_CONFIG_CONTROLLER_SETTINGS);
   FILE* fd = fopen(szFile, "r");
   if ( NULL == fd )
   {
      log_softerror_and_alarm("Failed to load controller settings from file: %s (missing file). Resetted controller settings to default.", szFile);
      reset_ControllerSettings();
      save_ControllerSettings();
      return 0;
   }

   int iDummy = 0;
   int failed = 0;
   int iWriteOptionalValues = 0;
   char szBuff[256];
   szBuff[0] = 0;
   if ( 1 != fscanf(fd, "%s", szBuff) )
      failed = 1;

   if ( failed )
   {
      fclose(fd);
      log_softerror_and_alarm("Failed to load controller settings from file: %s (can't read config file version)", szFile);
      reset_ControllerSettings();
      save_ControllerSettings();
      return 0;
   }

   if ( 3 != fscanf(fd, "%d %d %d", &s_CtrlSettings.iDeveloperMode, &s_CtrlSettings.iUseBrokenVideoCRC, &s_CtrlSettings.iHDMIBoost) )
      { failed = 1; log_softerror_and_alarm("Load ctrl settings, failed on line 1"); }

   if ( 3 != fscanf(fd, "%d %d %d", &s_CtrlSettings.iOverVoltage, &s_CtrlSettings.iFreqARM, &s_CtrlSettings.iFreqGPU) )
      { failed = 1; log_softerror_and_alarm("Load ctrl settings, failed on line 2"); }

   if ( 5 != fscanf(fd, "%d %d %d %d %d", &s_CtrlSettings.iNiceRouter, &s_CtrlSettings.iNiceCentral, &s_CtrlSettings.iNiceRXVideo, &s_CtrlSettings.ioNiceRouter, &s_CtrlSettings.ioNiceRXVideo) )
      { failed = 1; log_softerror_and_alarm("Load ctrl settings, failed on line 3"); }


   if ( 3 != fscanf(fd, "%*s %d %d %d", &s_CtrlSettings.iVideoForwardUSBType, &s_CtrlSettings.iVideoForwardUSBPort, &s_CtrlSettings.iVideoForwardUSBPacketSize) )
      { failed = 1; log_softerror_and_alarm("Load ctrl settings, failed on line 4"); }

   if ( 3 != fscanf(fd, "%*s %d %d %d", &s_CtrlSettings.iTelemetryForwardUSBType, &s_CtrlSettings.iTelemetryForwardUSBPort, &s_CtrlSettings.iTelemetryForwardUSBPacketSize) )
      { failed = 1; log_softerror_and_alarm("Load ctrl settings, failed on line 5"); }

   if ( 2 != fscanf(fd, "%d %d", &s_CtrlSettings.iTelemetryOutputSerialPortIndex, &s_CtrlSettings.iTelemetryInputSerialPortIndex) )
      { failed = 1; log_softerror_and_alarm("Load ctrl settings, failed on line 6"); }

   if ( 2 != fscanf(fd, "%d %d", &s_CtrlSettings.iMAVLinkSysIdController, &s_CtrlSettings.iDisableHDMIOverscan) )
      { failed = 1; log_softerror_and_alarm("Load ctrl settings, failed on line 7"); }

   if ( 2 != fscanf(fd, "%d %d", &s_CtrlSettings.iRenderFPS, &s_CtrlSettings.iShowVoltage) )
      { failed = 1; log_softerror_and_alarm("Load ctrl settings, failed on line 8"); }

   if ( 2 != fscanf(fd, "%d %d", &s_CtrlSettings.nRetryRetransmissionAfterTimeoutMS, &s_CtrlSettings.nRequestRetransmissionsOnVideoSilenceMs) )
      { failed = 1; log_softerror_and_alarm("Load ctrl settings, failed on line 9"); }
   
   if ( 2 != fscanf(fd, "%d %u", &s_CtrlSettings.nUseFixedIP, &s_CtrlSettings.uFixedIP) )
      { failed = 1; log_softerror_and_alarm("Load ctrl settings, failed on line 10"); }

   if ( 3 != fscanf(fd, "%*s %d %d %d", &s_CtrlSettings.nVideoForwardETHType, &s_CtrlSettings.nVideoForwardETHPort, &s_CtrlSettings.nVideoForwardETHPacketSize) )
      { failed = 1; log_softerror_and_alarm("Load ctrl settings, failed on line 11"); }

   if ( 3 != fscanf(fd, "%d %d %d", &s_CtrlSettings.nAutomaticTxCard, &s_CtrlSettings.nRotaryEncoderFunction, &s_CtrlSettings.nRotaryEncoderSpeed) )
      { failed = 1; log_softerror_and_alarm("Load ctrl settings, failed on line 12"); }

   if ( 3 != fscanf(fd, "%d %d %d", &s_CtrlSettings.nPingClockSyncFrequency, &s_CtrlSettings.nGraphRadioRefreshInterval, &s_CtrlSettings.nGraphVideoRefreshInterval) )
      { failed = 1; log_softerror_and_alarm("Load ctrl settings, failed on line 13"); }

   // Extended values

   if ( 2 != fscanf(fd, "%d %d", &s_CtrlSettings.iDisableRetransmissionsAfterControllerLinkLostMiliseconds, &s_CtrlSettings.iVideoDecodeStatsSnapshotClosesOnTimeout) )
      { failed = 1; log_softerror_and_alarm("Load ctrl settings, failed on line 14"); }

   if ( 2 != fscanf(fd, "%d %d", &s_CtrlSettings.nRotaryEncoderFunction2, &s_CtrlSettings.nRotaryEncoderSpeed2) )
      { failed = 1; log_softerror_and_alarm("Load ctrl settings, failed on line 15"); }

   if ( 2 != fscanf(fd, "%d %d", &iDummy, &s_CtrlSettings.iFreezeOSD) )
      { failed = 1; log_softerror_and_alarm("Load ctrl settings, failed on line 16"); }

   if ( 2 != fscanf(fd, "%d %d", &s_CtrlSettings.iDevSwitchVideoProfileUsingQAButton, &s_CtrlSettings.iShowControllerAdaptiveInfoStats) )
      { failed = 1; log_softerror_and_alarm("Load ctrl settings, failed on line 17"); }

   if ( 1 != fscanf(fd, "%d", &s_CtrlSettings.iShowVideoStreamInfoCompactType) )
      { failed = 1; log_softerror_and_alarm("Load ctrl settings, failed on line 18"); }

   if ( 4 != fscanf(fd, "%d %d %d %d", &s_CtrlSettings.iSearchSiKAirRate, &s_CtrlSettings.iSearchSiKECC, &s_CtrlSettings.iSearchSiKLBT, &s_CtrlSettings.iSearchSiKMCSTR) )
      { failed = 1; log_softerror_and_alarm("Load ctrl settings, failed on line 19"); }


   if ( 2 != fscanf(fd, "%d %d", &s_CtrlSettings.iAudioOutputDevice, &s_CtrlSettings.iAudioOutputVolume) )
      { failed = 1; log_softerror_and_alarm("Load ctrl settings, failed on line 20"); }

   if ( 2 != fscanf(fd, "%d %u", &s_CtrlSettings.iDevRxLoopTimeout, &s_CtrlSettings.uShowBigRxHistoryInterface) )
      { failed = 1; log_softerror_and_alarm("Load ctrl settings, failed on line 21"); }

   if ( 1 != fscanf(fd, "%d", &s_CtrlSettings.iSiKPacketSize) )
      { failed = 1; log_softerror_and_alarm("Load ctrl settings, failed on line 22"); }

   if ( 2 != fscanf(fd, "%d %d", &s_CtrlSettings.iRadioRxThreadPriority, &s_CtrlSettings.iRadioTxThreadPriority) )
      { failed = 1; log_softerror_and_alarm("Load ctrl settings, failed on line 23"); }

   if ( 3 != fscanf(fd, "%d %d %d", &s_CtrlSettings.iRadioTxUsesPPCAP, &s_CtrlSettings.iRadioBypassSocketBuffers, &s_CtrlSettings.iFixedTxPower) )
      { failed = 1; log_softerror_and_alarm("Load ctrl settings, failed on line 24"); }

   if ( 2 != fscanf(fd, "%d %d", &s_CtrlSettings.iCoresAdjustment, &s_CtrlSettings.iPrioritiesAdjustment) )
      { log_softerror_and_alarm("Load ctrl settings, failed on line 25");
        s_CtrlSettings.iCoresAdjustment = 1; s_CtrlSettings.iPrioritiesAdjustment = 1; }

   if ( 1 != fscanf(fd, "%d ", &s_CtrlSettings.iStreamerOutputMode) )
      { log_softerror_and_alarm("Load ctrl settings, failed on line 26");
        s_CtrlSettings.iStreamerOutputMode = 0;
      }
   if ( 1 != fscanf(fd, "%d ", &s_CtrlSettings.iVideoMPPBuffersSize) )
      { log_softerror_and_alarm("Load ctrl settings, failed on line 27");
         s_CtrlSettings.iVideoMPPBuffersSize = DEFAULT_MPP_BUFFERS_SIZE;
         iWriteOptionalValues = 1; }

   fclose(fd);

   //--------------------------------------------------------
   // Validate settings

   if ( (s_CtrlSettings.iStreamerOutputMode < 0) || (s_CtrlSettings.iStreamerOutputMode > 2) )
      s_CtrlSettings.iStreamerOutputMode = 0;

   if ( (s_CtrlSettings.iVideoMPPBuffersSize < 5) || (s_CtrlSettings.iVideoMPPBuffersSize > 128) )
      s_CtrlSettings.iVideoMPPBuffersSize = DEFAULT_MPP_BUFFERS_SIZE;
     
   if ( s_CtrlSettings.iMAVLinkSysIdController <= 0 || s_CtrlSettings.iMAVLinkSysIdController > 255 )
      s_CtrlSettings.iMAVLinkSysIdController = DEFAULT_MAVLINK_SYS_ID_CONTROLLER;
   if ( s_CtrlSettings.iVideoForwardUSBType < 0 || s_CtrlSettings.iVideoForwardUSBType > 1 || s_CtrlSettings.iVideoForwardUSBPacketSize == 0 || s_CtrlSettings.iVideoForwardUSBPort == 0 )
      { s_CtrlSettings.iVideoForwardUSBType = 0; s_CtrlSettings.iVideoForwardUSBPort = 0; s_CtrlSettings.iVideoForwardUSBPacketSize = 1024; }

   if ( s_CtrlSettings.iTelemetryForwardUSBType < 0 || s_CtrlSettings.iTelemetryForwardUSBType > 1 ||  s_CtrlSettings.iTelemetryForwardUSBPort == 0 ||  s_CtrlSettings.iTelemetryForwardUSBPacketSize == 0 )
      { s_CtrlSettings.iTelemetryForwardUSBType = 0; s_CtrlSettings.iTelemetryForwardUSBPort = 0; s_CtrlSettings.iTelemetryForwardUSBPacketSize = 1024; }

   if ( s_CtrlSettings.iNiceRouter < -18 || s_CtrlSettings.iNiceRouter > 0 )
      s_CtrlSettings.iNiceRouter = DEFAULT_PRIORITY_PROCESS_ROUTER;
   if ( s_CtrlSettings.iNiceRXVideo < -18 || s_CtrlSettings.iNiceRXVideo > 0 )
      s_CtrlSettings.iNiceRXVideo = DEFAULT_PRIORITY_PROCESS_VIDEO_RX;
   if ( s_CtrlSettings.iNiceCentral < -16 || s_CtrlSettings.iNiceCentral > 0 )
      s_CtrlSettings.iNiceCentral = DEFAULT_PRIORITY_PROCESS_CENTRAL;
     
   if ( s_CtrlSettings.iRenderFPS < 10 || s_CtrlSettings.iRenderFPS > 30 )
      s_CtrlSettings.iRenderFPS = 10;

   if ( s_CtrlSettings.nRotaryEncoderFunction < 0 || s_CtrlSettings.nRotaryEncoderFunction > 2 )
      s_CtrlSettings.nRotaryEncoderFunction = 1;
   if ( s_CtrlSettings.nRotaryEncoderSpeed < 0 || s_CtrlSettings.nRotaryEncoderSpeed > 1 )
      s_CtrlSettings.nRotaryEncoderSpeed = 0;

   if ( s_CtrlSettings.nPingClockSyncFrequency < 1 || s_CtrlSettings.nPingClockSyncFrequency > 50 )
      s_CtrlSettings.nPingClockSyncFrequency = DEFAULT_PING_FREQUENCY;

   if ( (s_CtrlSettings.iSiKPacketSize < 10) || (s_CtrlSettings.iSiKPacketSize > 250 ) )
      s_CtrlSettings.iSiKPacketSize = DEFAULT_SIK_PACKET_SIZE;

   if ( failed )
   {
      log_line("Invalid settings file %s, error code: %d. Reseted to default.", szFile, failed);
      reset_ControllerSettings();
      save_ControllerSettings();
   }
   else if ( 1 == iWriteOptionalValues )
   {
      log_line("Incomplete settings file %s, write settings again.", szFile);
      save_ControllerSettings();    
   }
   else
      log_line("Loaded controller settings from file: %s", szFile);
   return 1;
}

ControllerSettings* get_ControllerSettings()
{
   if ( ! s_CtrlSettingsLoaded )
      load_ControllerSettings();
   return &s_CtrlSettings;
}

u32 compute_ping_interval_ms(u32 uModelFlags, u32 uRxTxSyncType, u32 uCurrentVideoProfileFlags)
{
   u32 ping_interval_ms = 1000/DEFAULT_PING_FREQUENCY;
   if ( s_CtrlSettings.nPingClockSyncFrequency != 0 )
      ping_interval_ms = 1000/s_CtrlSettings.nPingClockSyncFrequency;

   if ( uModelFlags & MODEL_FLAG_PRIORITIZE_UPLINK )
      ping_interval_ms = (ping_interval_ms * 80) / 100;

   return ping_interval_ms;
}

#else
int save_ControllerSettings() { return 0; }
int load_ControllerSettings() { return 0; }
void reset_ControllerSettings() {}
ControllerSettings* get_ControllerSettings() { return NULL; }

u32 compute_ping_interval_ms(u32 uModelFlags, u32 uRxTxSyncType, u32 uCurrentVideoProfileFlags) { return 200000; }
#endif
