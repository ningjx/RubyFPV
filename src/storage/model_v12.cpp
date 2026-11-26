/*
    Ruby Licence
    Copyright (c) 2020-2025 Petru Soroaga petrusoroaga@yahoo.com
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

#include "../settings/models.h"
#include "../core/string_utils.h"

#define MODEL_FILE_STAMP_ID "vXI.7stm"

bool Model::loadVersion12(FILE* fd)
{
   char szBuff[256];
   int tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0, tmp5 = 0, tmp6 = 0;
   u32 u1 = 0, u2 = 0, u3 = 0, u4 = 0, u5 = 0, u6 = 0, u7 = 0;
   int vt = 0;
   bool bOk = true;

   if ( 1 != fscanf(fd, "%s", szBuff) )
      { bOk = false; log_softerror_and_alarm("11-0"); return false; }

   if ( 1 != fscanf(fd, "%*s %d", &iSaveCount) )
      { bOk = false; log_softerror_and_alarm("11-1"); return false; }

   if ( 5 != fscanf(fd, "%*s %u %u %u %u %u", &sw_version, &uVehicleId, &uModelFlags, &hwCapabilities.uBoardType, &alarms) )
      { bOk = false; log_softerror_and_alarm("11-2"); return false; }

   if ( hardware_is_vehicle() )
      sw_version = (SYSTEM_SW_VERSION_MAJOR * 256 + SYSTEM_SW_VERSION_MINOR) | (SYSTEM_SW_BUILD_NUMBER<<16);

   if ( 2 != fscanf(fd, "%*s %u %u", &uControllerId, &uControllerBoardType ) )
      { bOk = false; log_softerror_and_alarm("11-3"); return false; }

   if ( 1 != fscanf(fd, "%s", vehicle_name) )
      { bOk = false; log_softerror_and_alarm("11-4"); return false; }
   if ( vehicle_name[0] == '*' && vehicle_name[1] == 0 )
      vehicle_name[0] = 0;

   for( int i=0; i<(int)strlen(vehicle_name); i++ )
      if ( vehicle_name[i] == '_' )
         vehicle_name[i] = ' ';

   str_sanitize_modelname(vehicle_name);

   if ( 2 != fscanf(fd, "%d %u", &rxtx_sync_type, &camera_rc_channels ) )
      { bOk = false; log_softerror_and_alarm("11-5"); return false; }

   if ( 4 != fscanf(fd, "%d %d %u %d", &tmp1, &vt, &m_Stats.uTotalFlightTime, &iGPSCount ) )
      { bOk = false; log_softerror_and_alarm("11-6"); return false; }

   is_spectator = (bool)tmp1;
   vehicle_type = vt;

   //----------------------------------------
   // CPU & processes 

   if ( 4 != fscanf(fd, "%*s %u %d %d %d", &processesPriorities.uProcessesFlags, &processesPriorities.iOverVoltage, &processesPriorities.iFreqARM, &processesPriorities.iFreqGPU) )
      { bOk = false; log_softerror_and_alarm("11-7"); return false; }
   if ( 2 != fscanf(fd, "%d %d", &processesPriorities.ioNiceRouter, &processesPriorities.ioNiceVideo) )
      { bOk = false; log_softerror_and_alarm("11-8"); return false; }
   if ( 7 != fscanf(fd, "%d %d %d %d %d %d %d", &processesPriorities.iThreadPriorityRouter, &processesPriorities.iThreadPriorityRadioRx,
     &processesPriorities.iThreadPriorityRadioTx, &processesPriorities.iThreadPriorityVideoCapture,
     &processesPriorities.iThreadPriorityRC, &processesPriorities.iThreadPriorityTelemetry,
     &processesPriorities.iThreadPriorityOthers) )
      { bOk = false; log_softerror_and_alarm("11-9"); return false; }
   if ( 7 != fscanf(fd, "%d %d %d %d %d %d %d", &processesPriorities.iCoreRadioRx, &processesPriorities.iCoreRouter, &processesPriorities.iCoreVideoCapture, &processesPriorities.iCoreTelemetry, &processesPriorities.iCoreCommands, &processesPriorities.iCoreRC, &processesPriorities.iCoreOthers) )
      { bOk = false; log_softerror_and_alarm("11-10"); return false; }

   //----------------------------------------
   // Radio interfaces

   if ( 1 != fscanf(fd, "%*s %d", &radioInterfacesParams.interfaces_count) )
      { bOk = false; log_softerror_and_alarm("11-11"); return false; }
   if ( (radioInterfacesParams.interfaces_count < 0) || (radioInterfacesParams.interfaces_count >= MAX_RADIO_INTERFACES) )
      { bOk = false; log_softerror_and_alarm("11-12"); return false; }

   for( int i=0; i<radioInterfacesParams.interfaces_count; i++ )
   {
      char szTmp[256];
      char szTmp2[256];
      if ( 3 != fscanf(fd, "%d %d %u", &(radioInterfacesParams.interface_card_model[i]), &(radioInterfacesParams.interface_link_id[i]), &(radioInterfacesParams.interface_current_frequency_khz[i])) )
         { bOk = false; log_softerror_and_alarm("11-13"); return false; }

      if ( 7 != fscanf(fd, "%u %d %u %u %d %s %s", &u1, &tmp1, &u2, &u3, &tmp2, szTmp2, szTmp) )
         { bOk = false; log_softerror_and_alarm("11-14"); return false; }
      radioInterfacesParams.interface_capabilities_flags[i] = u1;
      radioInterfacesParams.interface_supported_bands[i] = (u8)tmp1;
      radioInterfacesParams.interface_radiotype_and_driver[i] = u2;
      radioInterfacesParams.interface_supported_radio_flags[i] = u3;
      radioInterfacesParams.interface_raw_power[i] = tmp2;

      szTmp[sizeof(szTmp)/sizeof(szTmp[0]) - 1] = 0;
      szTmp2[sizeof(szTmp2)/sizeof(szTmp2[0]) - 1] = 0;
      strncpy(radioInterfacesParams.interface_szMAC[i], szTmp2, MAX_MAC_LENGTH-1);
      radioInterfacesParams.interface_szMAC[i][MAX_MAC_LENGTH-1] = 0;
      int iStrLen = strlen(radioInterfacesParams.interface_szMAC[i]);
      if ( (iStrLen > 1) && (radioInterfacesParams.interface_szMAC[i][iStrLen-1] == '-') )
         radioInterfacesParams.interface_szMAC[i][iStrLen-1] = 0;
      strncpy(radioInterfacesParams.interface_szPort[i], szTmp, MAX_RADIO_PORT_NAME_LENGTH-1);
      radioInterfacesParams.interface_szPort[i][MAX_RADIO_PORT_NAME_LENGTH-1] = 0;
      iStrLen = strlen(radioInterfacesParams.interface_szPort[i]);
      if ( (iStrLen > 1) && (radioInterfacesParams.interface_szPort[i][iStrLen-1] == '-') )
         radioInterfacesParams.interface_szPort[i][iStrLen-1] = 0;
   }

   if ( 5 != fscanf(fd, "%d %d %d %u %d", &tmp1, &radioInterfacesParams.iAutoVehicleTxPower, &radioInterfacesParams.iAutoControllerTxPower, &radioInterfacesParams.uFlagsRadioInterfaces, &radioInterfacesParams.iDummyR1) )
      { bOk = false; log_softerror_and_alarm("11-15"); return false; }
   enableDHCP = (bool)tmp1;

   //----------------------------------------
   // Radio interfaces runtime capab

   int iCountInterfaces = 0;
   if ( 2 != fscanf(fd, "%*s %d %d", &iCountInterfaces, &tmp1) )
      { bOk = false; log_softerror_and_alarm("12-11"); return false; }
   if ( iCountInterfaces != radioInterfacesParams.interfaces_count )
      { bOk = false; log_softerror_and_alarm("12-12"); return false; }
   radioInterfacesRuntimeCapab.uFlagsRuntimeCapab = tmp1;
   for( int i=0; i<radioInterfacesParams.interfaces_count; i++ )
   {
      int iCountValues = 0;
      if ( 4 != fscanf(fd, "%d %d %d %d", &tmp1, &(radioInterfacesRuntimeCapab.iMaxSupportedLegacyDataRate[i]), &(radioInterfacesRuntimeCapab.iMaxSupportedMCSDataRate[i]), &iCountValues) )
         { bOk = false; log_softerror_and_alarm("12-13"); return false; }
      radioInterfacesRuntimeCapab.uInterfaceFlags[i] = tmp1;
      for( int k=0; k<iCountValues; k++ )
      {
         if ( 1 != fscanf(fd, "%d", &tmp1) )
            { bOk = false; log_softerror_and_alarm("12-14"); return false; }
         if ( k < MODEL_MAX_STORED_RADIO_INTERFACE_QUALITIES_VALUES )
            radioInterfacesRuntimeCapab.iQualitiesLegacy[i][k] = tmp1;
      }
      for( int k=0; k<iCountValues; k++ )
      {
         if ( 1 != fscanf(fd, "%d", &tmp1) )
            { bOk = false; log_softerror_and_alarm("12-14"); return false; }
         if ( k < MODEL_MAX_STORED_RADIO_INTERFACE_QUALITIES_VALUES )
            radioInterfacesRuntimeCapab.iQualitiesMCS[i][k] = tmp1;
      }
      for( int k=0; k<iCountValues; k++ )
      {
         if ( 1 != fscanf(fd, "%d", &tmp1) )
            { bOk = false; log_softerror_and_alarm("12-14"); return false; }
         if ( k < MODEL_MAX_STORED_RADIO_INTERFACE_QUALITIES_VALUES )
            radioInterfacesRuntimeCapab.iMaxTxPowerMwLegacy[i][k] = tmp1;
      }
      for( int k=0; k<iCountValues; k++ )
      {
         if ( 1 != fscanf(fd, "%d", &tmp1) )
            { bOk = false; log_softerror_and_alarm("12-14"); return false; }
         if ( k < MODEL_MAX_STORED_RADIO_INTERFACE_QUALITIES_VALUES )
            radioInterfacesRuntimeCapab.iMaxTxPowerMwMCS[i][k] = tmp1;
      }
   }

   //---------------------------------------
   // Radio links

   if ( 1 != fscanf(fd, "%*s %d", &radioLinksParams.links_count) )
      { bOk = false; log_softerror_and_alarm("11-16"); return false; }
   if ( (radioLinksParams.links_count < 0) || (radioLinksParams.links_count >= MAX_RADIO_INTERFACES) )
      { bOk = false; log_softerror_and_alarm("11-17"); return false; }

   for( int i=0; i<radioLinksParams.links_count; i++ )
   {
      if ( 5 != fscanf(fd, "%u %u %u %d %d", &(radioLinksParams.link_frequency_khz[i]), &(radioLinksParams.link_capabilities_flags[i]), &(radioLinksParams.link_radio_flags_tx[i]), &(radioLinksParams.downlink_datarate_video_bps[i]), &(radioLinksParams.downlink_datarate_data_bps[i])) )
         { bOk = false; log_softerror_and_alarm("11-18"); return false; }

      if ( 5 != fscanf(fd, "%d %d %d %d %u", &tmp1, &(radioLinksParams.uplink_datarate_video_bps[i]), &(radioLinksParams.uplink_datarate_data_bps[i]), &tmp2, &(radioLinksParams.link_radio_flags_rx[i]) ) )
         { bOk = false; log_softerror_and_alarm("11-19"); return false; }
      radioLinksParams.uSerialPacketSize[i] = tmp1;
      radioLinksParams.uMaxLinkLoadPercent[i] = tmp2;
   }

   if ( 2 != fscanf(fd, "%d %u", &radioLinksParams.iSiKPacketSize, &radioLinksParams.uGlobalRadioLinksFlags) )
      { bOk = false;  log_softerror_and_alarm("11-20"); return false; }

   //-------------------------------
   // Relay params

   if ( 5 != fscanf(fd, "%*s %d %u %d %u %u", &relay_params.isRelayEnabledOnRadioLinkId, &(relay_params.uRelayFrequencyKhz), &tmp2, &relay_params.uRelayedVehicleId, &relay_params.uRelayCapabilitiesFlags) )
      { bOk = false; log_softerror_and_alarm("11-21"); return false; }
   relay_params.uCurrentRelayMode = tmp2;

   //----------------------------------------
   // Telemetry

   if ( 3 != fscanf(fd, "%*s %d %d %u", &telemetry_params.fc_telemetry_type, &telemetry_params.iUpdateRateHz, &telemetry_params.uDummyT1) )
      { bOk = false; log_softerror_and_alarm("11-22"); return false; }

   if ( 4 != fscanf(fd, "%d %d %d %u", &telemetry_params.iVideoBitrateHistoryGraphSampleInterval, &telemetry_params.vehicle_mavlink_id, &telemetry_params.controller_mavlink_id, &telemetry_params.flags ) )
      { bOk = false; log_softerror_and_alarm("11-23"); return false; }

   //----------------------------------------
   // Video

   if ( 4 != fscanf(fd, "%*s %d %d %u %u", &video_params.iCurrentVideoProfile, &video_params.iH264Slices, &video_params.uDummyV1, &video_params.lowestAllowedAdaptiveVideoBitrate) )
      { bOk = false; log_softerror_and_alarm("11-24"); return false; }
   
   if ( 5 != fscanf(fd, "%u %u %d %d %d", &video_params.uMaxAutoKeyframeIntervalMs, &video_params.uVideoExtraFlags, &video_params.iVideoWidth, &video_params.iVideoHeight, &video_params.iVideoFPS) )
      { bOk = false; log_softerror_and_alarm("11-25"); return false; }

   if ( 3 != fscanf(fd, "%d %d %d", &video_params.iRemovePPSVideoFrames, &video_params.iInsertPPSVideoFrames, &video_params.iInsertSPTVideoFramesTimings) )
      { bOk = false; log_softerror_and_alarm("11-26"); return false; }

   //--------------------------------------
   // Video link profiles

   if ( bOk && (1 != fscanf(fd, "%*s %d", &tmp1)) )
      { bOk = false; log_softerror_and_alarm("11-27"); return false; }
   if ( tmp1 != MAX_VIDEO_LINK_PROFILES )
      { bOk = false; log_softerror_and_alarm("11-28"); return false; }

   for( int i=0; i<MAX_VIDEO_LINK_PROFILES; i++ )
   {
      if ( 7 != fscanf(fd, "%u %u %u %d %u %d %d", &(video_link_profiles[i].uProfileFlags), &(video_link_profiles[i].uProfileEncodingFlags), &(video_link_profiles[i].uTargetVideoBitrateBPS), &(video_link_profiles[i].iAdaptiveAdjustmentStrength), &(video_link_profiles[i].uAdaptiveWeights), &(video_link_profiles[i].iDefaultFPS), &(video_link_profiles[i].iECPercentage)) )
         { bOk = false; log_softerror_and_alarm("11-29 %d", i); return false; }

      if ( is_sw_version_atleast(this, 11, 6) )
      {
         if ( 3 != fscanf(fd, "%d %u %u", &(video_link_profiles[i].iDefaultLinkLoad), &(video_link_profiles[i].uDummyVP1), &(video_link_profiles[i].uDummyVP2)) )
            { bOk = false; log_softerror_and_alarm("11-30 %d", i); return false; }
      }
      else
      {
         video_link_profiles[i].iDefaultLinkLoad = 0;
         video_link_profiles[i].uDummyVP1 = 0;
         video_link_profiles[i].uDummyVP2 = 0;
      }
      if ( 4 != fscanf(fd, "%d %d %d %d", &(video_link_profiles[i].iBlockDataPackets), &(video_link_profiles[i].iBlockECs), &(video_link_profiles[i].video_data_length), &(video_link_profiles[i].iKeyframeMS)) )
         { bOk = false; log_softerror_and_alarm("11-31 %d", i); return false; }
      
      if ( 5 != fscanf(fd, "%d %d %d %d %d", &(video_link_profiles[i].h264profile), &(video_link_profiles[i].h264level), &(video_link_profiles[i].h264refresh), &(video_link_profiles[i].h264quantization), &(video_link_profiles[i].iIPQuantizationDelta)) )
         { bOk = false; log_softerror_and_alarm("11-32 %d", i); return false; }
   }


   //----------------------------------------
   // Camera params

   if ( 2 != fscanf(fd, "%*s %d %d", &iCameraCount, &iCurrentCamera) )
      { bOk = false; log_softerror_and_alarm("11-33"); return false; }

   for( int k=0; k<MODEL_MAX_CAMERAS; k++ )
   {

      // Camera type:
      if ( 3 != fscanf(fd, "%*s %d %d %d", &(camera_params[k].iCameraType), &(camera_params[k].iForcedCameraType), &(camera_params[k].iCurrentProfile)) )
         { bOk = false; log_softerror_and_alarm("11-34"); return false; }

      //----------------------------------------
      // Camera sensor name

      char szTmp[1024];
      if ( 1 != fscanf(fd, "%*s %s", szTmp) )
         { bOk = false; log_softerror_and_alarm("11-35"); camera_params[k].szCameraName[0] = 0; return false; }
      else
      {
         szTmp[MAX_CAMERA_NAME_LENGTH-1] = 0;
         strcpy(camera_params[k].szCameraName, szTmp);
         camera_params[k].szCameraName[MAX_CAMERA_NAME_LENGTH-1] = 0;

         if ( camera_params[k].szCameraName[0] == '*' && camera_params[k].szCameraName[1] == 0 )
            camera_params[k].szCameraName[0] = 0;
         for( int i=0; i<(int)strlen(camera_params[k].szCameraName); i++ )
            if ( camera_params[k].szCameraName[i] == '*' )
               camera_params[k].szCameraName[i] = ' ';
      }

      for( int i=0; i<MODEL_CAMERA_PROFILES; i++ )
      {
         if ( 1 != fscanf(fd, "%*s %d", &camera_params[k].profiles[i].uFlags) )
            { bOk = false; log_softerror_and_alarm("11-36 %d", i); return false; }
         if ( 1 != fscanf(fd, "%d", &tmp1) )
            { bOk = false; log_softerror_and_alarm("11-37 %d", i); return false; }
         camera_params[k].profiles[i].flip_image = (bool)tmp1;

         if ( 4 != fscanf(fd, "%d %d %d %d", &tmp1, &tmp2, &tmp3, &tmp4) )
            { bOk = false; log_softerror_and_alarm("11-38 %d", i); return false; }
         camera_params[k].profiles[i].brightness = tmp1;
         camera_params[k].profiles[i].contrast = tmp2;
         camera_params[k].profiles[i].saturation = tmp3;
         camera_params[k].profiles[i].sharpness = tmp4;

         if ( 4 != fscanf(fd, "%d %d %d %d", &tmp1, &tmp2, &tmp3, &tmp4) )
            { bOk = false; log_softerror_and_alarm("11-39 %d", i); return false; }
         camera_params[k].profiles[i].exposure = tmp1;
         camera_params[k].profiles[i].whitebalance = tmp2;
         camera_params[k].profiles[i].metering = tmp3;
         camera_params[k].profiles[i].drc = tmp4;

         if ( 3 != fscanf(fd, "%f %f %f", &(camera_params[k].profiles[i].analogGain), &(camera_params[k].profiles[i].awbGainB), &(camera_params[k].profiles[i].awbGainR)) )
            { bOk = false; log_softerror_and_alarm("11-40 %d", i); return false; }

         if ( 2 != fscanf(fd, "%f %f", &(camera_params[k].profiles[i].fovH), &(camera_params[k].profiles[i].fovV)) )
            { bOk = false; log_softerror_and_alarm("11-41 %d", i); return false; }

         if ( 4 != fscanf(fd, "%d %d %d %d", &tmp1, &tmp2, &tmp3, &tmp4) )
            { bOk = false; log_softerror_and_alarm("11-42 %d", i); return false; }
         camera_params[k].profiles[i].vstab = tmp1;
         camera_params[k].profiles[i].ev = tmp2;
         camera_params[k].profiles[i].iso = tmp3;
         camera_params[k].profiles[i].iShutterSpeed = tmp4;

         if ( 3 != fscanf(fd, "%d %d %d", &tmp1, &tmp2, &tmp3) )
            { bOk = false; log_softerror_and_alarm("11-43 %d", i); return false; }

         camera_params[k].profiles[i].wdr = (u8)tmp1;
         camera_params[k].profiles[i].dayNightMode = (u8)tmp2;
         camera_params[k].profiles[i].hue = (u8)tmp3;

         if ( 1 != fscanf(fd, "%u", &u1) )
            { bOk = false; log_softerror_and_alarm("11-44 %d", i); return false; }
         camera_params[k].profiles[i].uDummyCamP = u1;
      }

   }

   for( int i=0; i<MODEL_MAX_CAMERAS; i++ )
   {
      camera_params[i].iCameraBinProfile = 0;
      camera_params[i].szCameraBinProfileName[0] = 0;

      char szTmpBin[MAX_CAMERA_BIN_PROFILE_NAME];
      if ( 2 != fscanf(fd, "%d %s", &camera_params[i].iCameraBinProfile, szTmpBin) )
         { bOk = false; log_softerror_and_alarm("11-45 %d", i); return false; }

      strncpy(camera_params[i].szCameraBinProfileName, szTmpBin, MAX_CAMERA_BIN_PROFILE_NAME-1);
      camera_params[i].szCameraBinProfileName[MAX_CAMERA_BIN_PROFILE_NAME-1] = 0;
      if ( '-' == camera_params[i].szCameraBinProfileName[0] )
         camera_params[i].szCameraBinProfileName[0] = 0;
   }

   //----------------------------------------
   // Audio Settings

   if ( 1 != fscanf(fd, "%*s %d", &tmp1) )
      { bOk = false; log_softerror_and_alarm("11-46"); return false; }

   audio_params.has_audio_device = (bool)tmp1;

   if ( 4 != fscanf(fd, "%d %d %d %u", &tmp2, &audio_params.volume, &audio_params.quality, &audio_params.uFlags) )
      { bOk = false; log_softerror_and_alarm("11-47"); return false; }

   audio_params.has_audio_device = tmp1;
   audio_params.enabled = tmp2;

   if ( 3 != fscanf(fd, "%d %d %u", &tmp1, &tmp2, &audio_params.uDummyA1) )
      { bOk = false; log_softerror_and_alarm("11-48"); return false; }
   audio_params.uECScheme = (u8)tmp1;
   audio_params.uPacketLength = (u16)tmp2;
   
   //----------------------------------------
   // Hardware info

   if ( 4 == fscanf(fd, "%*s %d %d %d %d", &hardwareInterfacesInfo.radio_interface_count, &hardwareInterfacesInfo.i2c_bus_count, &hardwareInterfacesInfo.i2c_device_count, &hardwareInterfacesInfo.serial_port_count) )
   {
      if ( hardwareInterfacesInfo.i2c_bus_count < 0 || hardwareInterfacesInfo.i2c_bus_count > MAX_MODEL_I2C_BUSSES )
         { log_softerror_and_alarm("11-49"); return false; }
      if ( hardwareInterfacesInfo.i2c_device_count < 0 || hardwareInterfacesInfo.i2c_device_count > MAX_MODEL_I2C_DEVICES )
         { log_softerror_and_alarm("11-50"); return false; }
      if ( hardwareInterfacesInfo.serial_port_count < 0 || hardwareInterfacesInfo.serial_port_count > MAX_MODEL_SERIAL_PORTS )
         { log_softerror_and_alarm("11-51"); return false; }

      for( int i=0; i<hardwareInterfacesInfo.i2c_bus_count; i++ )
         if ( 1 != fscanf(fd, "%d", &(hardwareInterfacesInfo.i2c_bus_numbers[i])) )
            { log_softerror_and_alarm("11-52"); return false; }

      for( int i=0; i<hardwareInterfacesInfo.i2c_device_count; i++ )
         if ( 2 != fscanf(fd, "%d %d", &(hardwareInterfacesInfo.i2c_devices_bus[i]), &(hardwareInterfacesInfo.i2c_devices_address[i])) )
            { log_softerror_and_alarm("11-53"); return false; }

      for( int i=0; i<hardwareInterfacesInfo.serial_port_count; i++ )
         if ( 3 != fscanf(fd, "%d %u %s", &(hardwareInterfacesInfo.serial_port_speed[i]), &(hardwareInterfacesInfo.serial_port_supported_and_usage[i]), &(hardwareInterfacesInfo.serial_port_names[i][0])) )
            { log_softerror_and_alarm("11-54"); return false; }
   }
   else
   {
      hardwareInterfacesInfo.radio_interface_count = 0;
      hardwareInterfacesInfo.i2c_bus_count = 0;
      hardwareInterfacesInfo.i2c_device_count = 0;
      hardwareInterfacesInfo.serial_port_count = 0;
      bOk = false;
      log_softerror_and_alarm("11-55");
      return false;
   }

   if ( 3 != fscanf(fd, "%d %d %u", &hwCapabilities.iMaxTxVideoBlocksBuffer, &hwCapabilities.iMaxTxVideoBlockPackets, &hwCapabilities.uHWFlags) )
      { log_softerror_and_alarm("11-56"); return false; }

   if ( 3 != fscanf(fd, "%u %u %u", &hwCapabilities.uRubyBaseVersion, &hwCapabilities.uDummyHW1, &hwCapabilities.uDummyHW2) )
      { log_softerror_and_alarm("11-57"); return false; }

   //----------------------------------------
   // OSD

   if ( 6 != fscanf(fd, "%*s %d %d %d %f %d %d", &tmp5, &osd_params.iCurrentOSDScreen, &tmp1, &osd_params.voltage_alarm, &tmp2, &tmp3) )
      { bOk = false; log_softerror_and_alarm("11-58"); return false; }
   if ( tmp5 != MODEL_MAX_OSD_SCREENS )
      { bOk = false; log_softerror_and_alarm("11-59"); return false; }
   osd_params.voltage_alarm_enabled = (bool)tmp1;
   osd_params.altitude_relative = (bool)tmp2;
   osd_params.show_gps_position = (bool)tmp3;

   if ( 6 != fscanf(fd, "%d %d %d %d %d %d", &osd_params.battery_show_per_cell,  &osd_params.battery_cell_count, &osd_params.battery_capacity_percent_alarm, &tmp1, &osd_params.home_arrow_rotate, &osd_params.iRadioInterfacesGraphRefreshIntervalMs) )
      { bOk = false; log_softerror_and_alarm("11-60"); return false; }
   osd_params.invert_home_arrow = (bool)tmp1;

   if ( 7 != fscanf(fd, "%d %d %d %d %d %d %d", &tmp1, &tmp2, &tmp3, &tmp4, &tmp5, &tmp6, &osd_params.ahi_warning_angle) )
      { bOk = false; log_softerror_and_alarm("11-61"); return false; }

   osd_params.show_overload_alarm = (bool)tmp1;
   osd_params.show_stats_rx_detailed = (bool)tmp2;
   osd_params.show_stats_decode = (bool)tmp3;
   osd_params.show_stats_rc = (bool)tmp4;
   osd_params.show_full_stats = (bool)tmp5;
   osd_params.show_instruments = (bool)tmp6;

   for( int i=0; i<MODEL_MAX_OSD_SCREENS; i++ )
   {
      if ( 6 != fscanf(fd, "%u %u %u %u %u %d", &(osd_params.osd_flags[i]), &(osd_params.osd_flags2[i]), &(osd_params.osd_flags3[i]), &(osd_params.instruments_flags[i]), &(osd_params.osd_preferences[i]), &tmp1) )
         { bOk = false; log_softerror_and_alarm("11-62"); return false; }
      osd_params.osd_layout_preset[i] = (u8)tmp1;
   }

   if ( 1 != fscanf(fd, "%u", &osd_params.uFlags) )
      { bOk = false; log_softerror_and_alarm("11-63"); return false; }

   //----------------------------------------
   // RC

   if ( 4 != fscanf(fd, "%*s %d %d %d %d", &tmp1, &tmp2, &rc_params.receiver_type, &rc_params.rc_frames_per_second ) )
      { bOk = false; log_softerror_and_alarm("11-64"); return false; }
   if ( tmp1 != MAX_RC_CHANNELS )
      { bOk = false; log_softerror_and_alarm("11-65"); return false; }

   if ( 5 != fscanf(fd, "%d %d %d %d %d", &rc_params.inputType, &tmp1, &tmp2, &tmp3, &tmp4 ) )
      { bOk = false; log_softerror_and_alarm("11-66"); return false; }
   
   for( int i=0; i<MAX_RC_CHANNELS; i++ )
   {
      if ( 7 != fscanf(fd, "%u %u %u %u %u %u %u", &u1, &u2, &u3, &u4, &u5, &u6, &u7) )
         { bOk = false; log_softerror_and_alarm("11-67"); return false; }
      rc_params.rcChAssignment[i] = u1;
      rc_params.rcChMid[i] = u2;
      rc_params.rcChMin[i] = u3;
      rc_params.rcChMax[i] = u4;
      rc_params.rcChFailSafe[i] = u5;
      rc_params.rcChExpo[i] = u6;
      rc_params.rcChFlags[i] = u7;
   }
   
   if ( 3 != fscanf(fd, "%d %u %u", &rc_params.rc_failsafe_timeout_ms, &rc_params.failsafeFlags, &rc_params.channelsCount ) )
      { bOk = false; log_softerror_and_alarm("11-68"); return false; }
   
   if ( 4 != fscanf(fd, "%u %u %u %d", &rc_params.hid_id,  &rc_params.uRCFlags, &rc_params.rcChAssignmentThrotleReverse, &rc_params.iRCTranslationType) )
      { bOk = false; log_softerror_and_alarm("11-69"); return false; }

   //----------------------------------------
   // Misc

   if ( 4 != fscanf(fd, "%*s %u %u %u %d", &uModelPersistentStatusFlags, &uDeveloperFlags, &enc_flags, &tmp1) )
      { bOk = false; log_softerror_and_alarm("11-70"); uDeveloperFlags = (((u32)DEFAULT_DELAY_WIFI_CHANGE)<<DEVELOPER_FLAGS_WIFI_GUARD_DELAY_MASK_SHIFT); return false; }

   alarms_params.uAlarmMotorCurrentThreshold = tmp1;
  
   if ( 5 != fscanf(fd, "%*s %u %u %u %u %u", &m_Stats.uTotalFlights, &m_Stats.uCurrentOnTime, &m_Stats.uCurrentFlightTime, &m_Stats.uCurrentFlightDistance, &m_Stats.uCurrentFlightTotalCurrent) )
      { bOk = false; log_softerror_and_alarm("11-71"); return false;}
   if ( 5 != fscanf(fd, "%u %u %u %u %u", &m_Stats.uCurrentTotalCurrent, &m_Stats.uCurrentMaxAltitude, &m_Stats.uCurrentMaxDistance, &m_Stats.uCurrentMaxCurrent, &m_Stats.uCurrentMinVoltage) )
      { bOk = false; log_softerror_and_alarm("11-72"); return false;}
   if ( 3 != fscanf(fd, "%u %u %u", &m_Stats.uTotalOnTime, &m_Stats.uTotalFlightTime, &m_Stats.uTotalFlightDistance) )
      { bOk = false; log_softerror_and_alarm("11-73"); return false;}
   if ( 5 != fscanf(fd, "%u %u %u %u %u", &m_Stats.uTotalTotalCurrent, &m_Stats.uTotalMaxAltitude, &m_Stats.uTotalMaxDistance, &m_Stats.uTotalMaxCurrent, &m_Stats.uTotalMinVoltage) )
      { bOk = false; log_softerror_and_alarm("11-74"); return false;}
  
   //----------------------------------------
   // Functions & Triggers

   if ( 3 != fscanf(fd, "%*s %d %d %d", &tmp1, &tmp2, &tmp3 ) )
      { bOk = false; log_softerror_and_alarm("11-75"); return false; }

   functions_params.bEnableRCTriggerFreqSwitchLink1 = (bool)tmp1;
   functions_params.bEnableRCTriggerFreqSwitchLink2 = (bool)tmp2;
   functions_params.bEnableRCTriggerFreqSwitchLink3 = (bool)tmp3;


   if ( 3 != fscanf(fd, "%d %d %d", &functions_params.iRCTriggerChannelFreqSwitchLink1, &functions_params.iRCTriggerChannelFreqSwitchLink2, &functions_params.iRCTriggerChannelFreqSwitchLink3 ) )
      { bOk = false; log_softerror_and_alarm("11-76"); return false; }

   if ( 3 != fscanf(fd, "%d %d %d", &tmp1, &tmp2, &tmp3 ) )
      { bOk = false; log_softerror_and_alarm("11-77"); return false; }

   functions_params.bRCTriggerFreqSwitchLink1_is3Position = (bool)tmp1;
   functions_params.bRCTriggerFreqSwitchLink2_is3Position = (bool)tmp2;
   functions_params.bRCTriggerFreqSwitchLink3_is3Position = (bool)tmp3;

   for( int i=0; i<3; i++ )
   {
      if ( 6 != fscanf(fd, "%u %u %u %u %u %u", &functions_params.uChannels433FreqSwitch[i], &functions_params.uChannels868FreqSwitch[i], &functions_params.uChannels23FreqSwitch[i], &functions_params.uChannels24FreqSwitch[i], &functions_params.uChannels25FreqSwitch[i], &functions_params.uChannels58FreqSwitch[i]) )
         { bOk = false; log_softerror_and_alarm("11-78"); return false; }
   }

   for( unsigned int i=0; i<(sizeof(functions_params.uDummyF)/sizeof(functions_params.uDummyF[0])); i++ )
   {
      if ( 1 != fscanf(fd, "%u", &(functions_params.uDummyF[i])) )
         { bOk = false; log_softerror_and_alarm("11-79"); return false; }
   }

   //-----------------------------------
   // End of standard file

   if ( 1 != fscanf(fd, "%s", szBuff) )
      { bOk = false; log_softerror_and_alarm("11-82"); return false; }
   if ( 0 != strcmp(szBuff, "END_ST") )
      { bOk = false; log_softerror_and_alarm("11-83"); return false; }

   //----------------------------------------------------
   // Start of extra params, might be zero when loading older versions.

  
   if ( bOk )
   {
      if ( 1 != fscanf(fd, "%u", &uControllerBoardType) )
         uControllerBoardType = 0;
   }
   else
      uControllerBoardType = 0;

   //--------------------------------------------------
   // End reading file;
   //----------------------------------------

   // Validate settings;
   validate_settings();

   if ( telemetry_params.vehicle_mavlink_id <= 0 || telemetry_params.vehicle_mavlink_id > 255 )
      telemetry_params.vehicle_mavlink_id = DEFAULT_MAVLINK_SYS_ID_VEHICLE;
   if ( telemetry_params.controller_mavlink_id <= 0 || telemetry_params.controller_mavlink_id > 255 )
      telemetry_params.controller_mavlink_id = DEFAULT_MAVLINK_SYS_ID_CONTROLLER;
   if ( telemetry_params.flags == 0 )
      telemetry_params.flags = TELEMETRY_FLAGS_REQUEST_DATA_STREAMS | TELEMETRY_FLAGS_SPECTATOR_ENABLE;
   if ( rxtx_sync_type < 0 || rxtx_sync_type >= RXTX_SYNC_TYPE_LAST )
      rxtx_sync_type = RXTX_SYNC_TYPE_BASIC;

   /*
   log_line("---------------------------------------");
   log_line("Loaded radio links %d:", radioLinksParams.links_count);
   for( int i=0; i<radioLinksParams.links_count; i++ )
   {
      char szBuffR[128];
      str_get_radio_frame_flags_description(radioLinksParams.link_radio_flags_tx[i], szBuffR);
      log_line("Radio link %d frame flags: [%s]", i+1, szBuffR);
   }
   log_line("Loaded radio interfaces %d:", radioInterfacesParams.interfaces_count);
   for( int i=0; i<radioInterfacesParams.interfaces_count; i++ )
   {
      char szBuffR[128];
      str_get_radio_frame_flags_description(radioInterfacesParams.interface_supported_radio_flags[i], szBuffR);
      log_line("Radio interface %d frame flags: [%s]", i+1, szBuffR);
   }
   */
   return true;
}

bool Model::saveVersion12(FILE* fd, bool isOnController)
{
   char szSetting[256];
   char szModel[8096];

   szSetting[0] = 0;
   szModel[0] = 0;

   if ( ! isOnController )
      sw_version = (SYSTEM_SW_VERSION_MAJOR * 256 + SYSTEM_SW_VERSION_MINOR) | (SYSTEM_SW_BUILD_NUMBER<<16);


   sprintf(szSetting, "v 12\n"); // version number
   strcat(szModel, szSetting);
   sprintf(szSetting, "%s\n",MODEL_FILE_STAMP_ID);
   strcat(szModel, szSetting);
   sprintf(szSetting, "cnt: %d\n", iSaveCount);
   strcat(szModel, szSetting);
   sprintf(szSetting, "id: %u %u %u %u %u\n", sw_version, uVehicleId, uModelFlags, hwCapabilities.uBoardType, alarms);
   strcat(szModel, szSetting);

   sprintf(szSetting, "ctrl: %u %u\n", uControllerId, uControllerBoardType);
   strcat(szModel, szSetting);

   char szVeh[MAX_VEHICLE_NAME_LENGTH+1];
   strncpy(szVeh, vehicle_name, MAX_VEHICLE_NAME_LENGTH);
   szVeh[MAX_VEHICLE_NAME_LENGTH] = 0;
   str_sanitize_modelname(szVeh);
   for( int i=0; i<(int)strlen(szVeh); i++ )
   {
      if ( szVeh[i] == ' ' )
         szVeh[i] = '_';
   }

   if ( 0 == szVeh[0] )
      sprintf(szSetting, "*\n"); 
   else
      sprintf(szSetting, "%s\n", szVeh);
   strcat(szModel, szSetting);
 
   sprintf(szSetting, "%d %u ", rxtx_sync_type, camera_rc_channels );
   strcat(szModel, szSetting);
   sprintf(szSetting, "%d %d %u %d\n", is_spectator, vehicle_type, m_Stats.uTotalFlightTime, iGPSCount);
   strcat(szModel, szSetting);
   
   //----------------------------------------
   // CPU & processes 

   sprintf(szSetting, "cpu: %u %d %d %d\n", processesPriorities.uProcessesFlags, processesPriorities.iOverVoltage, processesPriorities.iFreqARM, processesPriorities.iFreqGPU);
   strcat(szModel, szSetting);
   sprintf(szSetting, "%d %d\n", processesPriorities.ioNiceRouter, processesPriorities.ioNiceVideo);
   strcat(szModel, szSetting);
   sprintf(szSetting, "%d %d %d %d %d %d %d\n",
     processesPriorities.iThreadPriorityRouter, processesPriorities.iThreadPriorityRadioRx,
     processesPriorities.iThreadPriorityRadioTx, processesPriorities.iThreadPriorityVideoCapture,
     processesPriorities.iThreadPriorityRC, processesPriorities.iThreadPriorityTelemetry,
     processesPriorities.iThreadPriorityOthers );
   strcat(szModel, szSetting);
   sprintf(szSetting, "%d %d %d %d %d %d %d\n", processesPriorities.iCoreRadioRx, processesPriorities.iCoreRouter, processesPriorities.iCoreVideoCapture, processesPriorities.iCoreTelemetry, processesPriorities.iCoreCommands, processesPriorities.iCoreRC, processesPriorities.iCoreOthers);
   strcat(szModel, szSetting);

   //----------------------------------------
   // Radio interfaces

   sprintf(szSetting, "radioint: %d\n", radioInterfacesParams.interfaces_count); 
   strcat(szModel, szSetting);
   for( int i=0; i<radioInterfacesParams.interfaces_count; i++ )
   {
      sprintf(szSetting, "%d %d %u\n", radioInterfacesParams.interface_card_model[i], radioInterfacesParams.interface_link_id[i], radioInterfacesParams.interface_current_frequency_khz[i]);
      strcat(szModel, szSetting);
      sprintf(szSetting, "%u %d %u %u %d %s- %s-\n", radioInterfacesParams.interface_capabilities_flags[i], radioInterfacesParams.interface_supported_bands[i], radioInterfacesParams.interface_radiotype_and_driver[i], radioInterfacesParams.interface_supported_radio_flags[i], radioInterfacesParams.interface_raw_power[i], radioInterfacesParams.interface_szMAC[i], radioInterfacesParams.interface_szPort[i]);
      strcat(szModel, szSetting);
   }
   sprintf(szSetting, "%d %d %d %u %d\n", enableDHCP, radioInterfacesParams.iAutoVehicleTxPower, radioInterfacesParams.iAutoControllerTxPower, radioInterfacesParams.uFlagsRadioInterfaces, radioInterfacesParams.iDummyR1);
   strcat(szModel, szSetting);

   //----------------------------------------
   // Radio interfaces runtime capab

   sprintf(szSetting, "radioint_capab: %d %d\n", radioInterfacesParams.interfaces_count, (int)radioInterfacesRuntimeCapab.uFlagsRuntimeCapab); 
   strcat(szModel, szSetting);
   for( int i=0; i<radioInterfacesParams.interfaces_count; i++ )
   {
      sprintf(szSetting, "%d %d %d %d\n", (int)radioInterfacesRuntimeCapab.uInterfaceFlags[i], radioInterfacesRuntimeCapab.iMaxSupportedLegacyDataRate[i], radioInterfacesRuntimeCapab.iMaxSupportedMCSDataRate[i], (int)MODEL_MAX_STORED_RADIO_INTERFACE_QUALITIES_VALUES);
      strcat(szModel, szSetting);
      for( int k=0; k<MODEL_MAX_STORED_RADIO_INTERFACE_QUALITIES_VALUES; k++ )
      {
         sprintf(szSetting, "%d ", radioInterfacesRuntimeCapab.iQualitiesLegacy[i][k]);
         strcat(szModel, szSetting);
      }
      strcat(szModel, "\n");
      for( int k=0; k<MODEL_MAX_STORED_RADIO_INTERFACE_QUALITIES_VALUES; k++ )
      {
         sprintf(szSetting, "%d ", radioInterfacesRuntimeCapab.iQualitiesMCS[i][k]);
         strcat(szModel, szSetting);
      }
      strcat(szModel, "\n");
      for( int k=0; k<MODEL_MAX_STORED_RADIO_INTERFACE_QUALITIES_VALUES; k++ )
      {
         sprintf(szSetting, "%d ", radioInterfacesRuntimeCapab.iMaxTxPowerMwLegacy[i][k]);
         strcat(szModel, szSetting);
      }
      strcat(szModel, "\n");
      for( int k=0; k<MODEL_MAX_STORED_RADIO_INTERFACE_QUALITIES_VALUES; k++ )
      {
         sprintf(szSetting, "%d ", radioInterfacesRuntimeCapab.iMaxTxPowerMwMCS[i][k]);
         strcat(szModel, szSetting);
      }
      strcat(szModel, "\n");
   }

   //----------------------------------------
   // Radio links

   sprintf(szSetting, "radiolinks: %d\n", radioLinksParams.links_count); 
   strcat(szModel, szSetting);

   for( int i=0; i<radioLinksParams.links_count; i++ )
   {
      sprintf( szSetting, "%u %u %u %d %d", radioLinksParams.link_frequency_khz[i], radioLinksParams.link_capabilities_flags[i], radioLinksParams.link_radio_flags_tx[i], radioLinksParams.downlink_datarate_video_bps[i], radioLinksParams.downlink_datarate_data_bps[i] );
      strcat(szModel, szSetting);
      sprintf( szSetting, " %d %d %d %d %u\n", (int)radioLinksParams.uSerialPacketSize[i], radioLinksParams.uplink_datarate_video_bps[i], radioLinksParams.uplink_datarate_data_bps[i], radioLinksParams.uMaxLinkLoadPercent[i], radioLinksParams.link_radio_flags_rx[i] );
      strcat(szModel, szSetting);
   }
   sprintf(szSetting, "%d %u\n", radioLinksParams.iSiKPacketSize, radioLinksParams.uGlobalRadioLinksFlags);
   strcat(szModel, szSetting);

   //---------------------------------
   // Relay params

   sprintf(szSetting, "relay: %d %u %d %u %u\n", relay_params.isRelayEnabledOnRadioLinkId, relay_params.uRelayFrequencyKhz, relay_params.uCurrentRelayMode, relay_params.uRelayedVehicleId, relay_params.uRelayCapabilitiesFlags); 
   strcat(szModel, szSetting);

   //----------------------------------------
   // Telemetry

   sprintf(szSetting, "telem: %d %d %u ", telemetry_params.fc_telemetry_type, telemetry_params.iUpdateRateHz, telemetry_params.uDummyT1);
   strcat(szModel, szSetting);
   sprintf(szSetting, "%d %d %d %u\n", telemetry_params.iVideoBitrateHistoryGraphSampleInterval, telemetry_params.vehicle_mavlink_id, telemetry_params.controller_mavlink_id, telemetry_params.flags);
   strcat(szModel, szSetting);
 
   //----------------------------------------
   // Video 

   sprintf(szSetting, "video: %d %d %u %u ", video_params.iCurrentVideoProfile, video_params.iH264Slices, video_params.uDummyV1, video_params.lowestAllowedAdaptiveVideoBitrate);
   strcat(szModel, szSetting);
 
   sprintf(szSetting, "%u %u %d %d %d\n", video_params.uMaxAutoKeyframeIntervalMs, video_params.uVideoExtraFlags, video_params.iVideoWidth, video_params.iVideoHeight, video_params.iVideoFPS);
   strcat(szModel, szSetting);
   
   sprintf(szSetting, "%d %d %d\n", video_params.iRemovePPSVideoFrames, video_params.iInsertPPSVideoFrames, video_params.iInsertSPTVideoFramesTimings);
   strcat(szModel, szSetting);

   //----------------------------------------
   // Video link profiles

   sprintf(szSetting, "video_prof: %d\n", (int)MAX_VIDEO_LINK_PROFILES);
   strcat(szModel, szSetting);
   for( int i=0; i<MAX_VIDEO_LINK_PROFILES; i++ )
   {
      sprintf(szSetting, "%u %u %u %d %u %d %d ", video_link_profiles[i].uProfileFlags, video_link_profiles[i].uProfileEncodingFlags, video_link_profiles[i].uTargetVideoBitrateBPS, video_link_profiles[i].iAdaptiveAdjustmentStrength, video_link_profiles[i].uAdaptiveWeights, video_link_profiles[i].iDefaultFPS, video_link_profiles[i].iECPercentage);
      strcat(szModel, szSetting);
      if ( is_sw_version_atleast(this, 11, 6) )
      {
         sprintf(szSetting, "%d %u %u\n", video_link_profiles[i].iDefaultLinkLoad, video_link_profiles[i].uDummyVP1, video_link_profiles[i].uDummyVP2);
         strcat(szModel, szSetting);
      }
      sprintf(szSetting, "%d %d %d %d ", video_link_profiles[i].iBlockDataPackets, video_link_profiles[i].iBlockECs, video_link_profiles[i].video_data_length, video_link_profiles[i].iKeyframeMS);
      strcat(szModel, szSetting);
      sprintf(szSetting, "%d %d %d %d %d\n", video_link_profiles[i].h264profile, video_link_profiles[i].h264level, video_link_profiles[i].h264refresh, video_link_profiles[i].h264quantization, video_link_profiles[i].iIPQuantizationDelta);
      strcat(szModel, szSetting);
   }

   //----------------------------------------
   // Camera params

   sprintf(szSetting, "cameras: %d %d\n", iCameraCount, iCurrentCamera); 
   strcat(szModel, szSetting);
   for( int k=0; k<MODEL_MAX_CAMERAS; k++ )
   {

      sprintf(szSetting, "camera_%d: %d %d %d\n", k, camera_params[k].iCameraType, camera_params[k].iForcedCameraType, camera_params[k].iCurrentProfile); 
      strcat(szModel, szSetting);

      //----------------------------------------
      // Camera sensor name

      char szTmp[MAX_CAMERA_NAME_LENGTH+1];
      strncpy(szTmp, camera_params[k].szCameraName, MAX_CAMERA_NAME_LENGTH);
      szTmp[MAX_CAMERA_NAME_LENGTH] = 0;
      for( int i=0; i<(int)strlen(szTmp); i++ )
      {
         if ( szTmp[i] == ' ' )
            szTmp[i] = '*';
      }

      if ( 0 == szTmp[0] )
         sprintf(szSetting, "cname: *\n"); 
      else
         sprintf(szSetting, "cname: %s\n", szTmp);
      strcat(szModel, szSetting);


      for( int i=0; i<MODEL_CAMERA_PROFILES; i++ )
      {
      sprintf(szSetting, "cam_profile_%d: %d %d ", i, camera_params[k].profiles[i].uFlags, camera_params[k].profiles[i].flip_image); 
      strcat(szModel, szSetting);
      sprintf(szSetting, "%d %d %d %d ", camera_params[k].profiles[i].brightness, camera_params[k].profiles[i].contrast, camera_params[k].profiles[i].saturation, camera_params[k].profiles[i].sharpness);
      strcat(szModel, szSetting);
      sprintf(szSetting, "%d %d %d %d\n", camera_params[k].profiles[i].exposure, camera_params[k].profiles[i].whitebalance, camera_params[k].profiles[i].metering, camera_params[k].profiles[i].drc);
      strcat(szModel, szSetting);
      sprintf(szSetting, "%f %f %f", camera_params[k].profiles[i].analogGain, camera_params[k].profiles[i].awbGainB, camera_params[k].profiles[i].awbGainR);
      strcat(szModel, szSetting);
      sprintf(szSetting, "%f %f ", camera_params[k].profiles[i].fovH, camera_params[k].profiles[i].fovV);
      strcat(szModel, szSetting);
      sprintf(szSetting, "%d %d %d %d\n", camera_params[k].profiles[i].vstab, camera_params[k].profiles[i].ev, camera_params[k].profiles[i].iso, camera_params[k].profiles[i].iShutterSpeed);
      strcat(szModel, szSetting);

      sprintf(szSetting, "%d ", (int)camera_params[k].profiles[i].wdr); 
      strcat(szModel, szSetting);

      sprintf(szSetting, "%d %d ", (int)camera_params[k].profiles[i].dayNightMode, (int)camera_params[k].profiles[i].hue);
      strcat(szModel, szSetting);

      sprintf(szSetting, " %u\n", camera_params[k].profiles[i].uDummyCamP); 
      strcat(szModel, szSetting);
      }
   }

   for( int i=0; i<MODEL_MAX_CAMERAS; i++ )
   {
      if ( i != 0 )
         strcat(szModel, " ");
      if ( 0 == camera_params[i].szCameraBinProfileName[0] )
         sprintf(szSetting, "%d -", camera_params[i].iCameraBinProfile);
      else
         sprintf(szSetting, "%d %s", camera_params[i].iCameraBinProfile, camera_params[i].szCameraBinProfileName);
      strcat(szModel, szSetting);
   }
   strcat(szModel, "\n");

   //----------------------------------------
   // Audio

   sprintf(szSetting, "audio: %d ", (int)audio_params.has_audio_device);
   strcat(szModel, szSetting);
   sprintf(szSetting, "%d %d %d %u ", (int)audio_params.enabled, audio_params.volume, audio_params.quality, audio_params.uFlags);
   strcat(szModel, szSetting);
   sprintf(szSetting, "%d %d %u\n", (int)audio_params.uECScheme, (int)audio_params.uPacketLength, audio_params.uDummyA1);
   strcat(szModel, szSetting);

   //----------------------------------------
   // Hardware Info

   sprintf(szSetting, "hw_info: %d %d %d %d\n", hardwareInterfacesInfo.radio_interface_count, hardwareInterfacesInfo.i2c_bus_count, hardwareInterfacesInfo.i2c_device_count, hardwareInterfacesInfo.serial_port_count);
   strcat(szModel, szSetting);

   for( int i=0; i<hardwareInterfacesInfo.i2c_bus_count; i++ )
   {
      sprintf(szSetting, " %d", hardwareInterfacesInfo.i2c_bus_numbers[i]);
      strcat(szModel, szSetting);
   }
   sprintf(szSetting, "\n");
   strcat(szModel, szSetting);

   for( int i=0; i<hardwareInterfacesInfo.i2c_device_count; i++ )
   {
      sprintf(szSetting, "%d %d\n", hardwareInterfacesInfo.i2c_devices_bus[i], hardwareInterfacesInfo.i2c_devices_address[i]);
      strcat(szModel, szSetting);
   }

   for( int i=0; i<hardwareInterfacesInfo.serial_port_count; i++ )
   {
      sprintf(szSetting, "%d %u %s\n", hardwareInterfacesInfo.serial_port_speed[i], hardwareInterfacesInfo.serial_port_supported_and_usage[i], hardwareInterfacesInfo.serial_port_names[i]);
      strcat(szModel, szSetting);
   }

   sprintf(szSetting, "%d %d %u %u %u %u\n", hwCapabilities.iMaxTxVideoBlocksBuffer, hwCapabilities.iMaxTxVideoBlockPackets, hwCapabilities.uHWFlags, hwCapabilities.uRubyBaseVersion, hwCapabilities.uDummyHW1, hwCapabilities.uDummyHW2);
   strcat(szModel, szSetting);

   //----------------------------------------
   // OSD 

   sprintf(szSetting, "osd: %d %d %d %f %d %d\n", MODEL_MAX_OSD_SCREENS, osd_params.iCurrentOSDScreen, osd_params.voltage_alarm_enabled, osd_params.voltage_alarm, osd_params.altitude_relative, osd_params.show_gps_position); 
   strcat(szModel, szSetting);
   sprintf(szSetting, "%d %d %d %d %d %d\n", osd_params.battery_show_per_cell, osd_params.battery_cell_count, osd_params.battery_capacity_percent_alarm, osd_params.invert_home_arrow, osd_params.home_arrow_rotate, osd_params.iRadioInterfacesGraphRefreshIntervalMs);
   strcat(szModel, szSetting);

   sprintf(szSetting, "%d %d %d %d %d %d %d\n", osd_params.show_overload_alarm, osd_params.show_stats_rx_detailed, osd_params.show_stats_decode, osd_params.show_stats_rc, osd_params.show_full_stats, osd_params.show_instruments, osd_params.ahi_warning_angle);
   strcat(szModel, szSetting);
   for( int i=0; i<MODEL_MAX_OSD_SCREENS; i++ )
   {
      sprintf(szSetting, "%u %u %u %u %u %d\n", osd_params.osd_flags[i], osd_params.osd_flags2[i], osd_params.osd_flags3[i], osd_params.instruments_flags[i], osd_params.osd_preferences[i], osd_params.osd_layout_preset[i]);
      strcat(szModel, szSetting);
   }
   sprintf(szSetting, "%u\n", osd_params.uFlags);
   strcat(szModel, szSetting);

   //----------------------------------------
   // RC 

   sprintf(szSetting, "rc: %d %d %d %d ", MAX_RC_CHANNELS, 0, rc_params.receiver_type, rc_params.rc_frames_per_second);
   strcat(szModel, szSetting);
   sprintf(szSetting, "%d %d %d %d %d\n", rc_params.inputType, 0,0,0,0);
   strcat(szModel, szSetting);
   for( int i=0; i<MAX_RC_CHANNELS; i++ )
   {
      sprintf(szSetting, "%u %u %u %u %u %u %u\n", (u32)rc_params.rcChAssignment[i], (u32)rc_params.rcChMid[i], (u32)rc_params.rcChMin[i], (u32)rc_params.rcChMax[i], (u32)rc_params.rcChFailSafe[i], (u32)rc_params.rcChExpo[i], (u32)rc_params.rcChFlags[i]);
      strcat(szModel, szSetting);
   }

   sprintf(szSetting, "%d %u %u ", rc_params.rc_failsafe_timeout_ms, rc_params.failsafeFlags, rc_params.channelsCount );
   strcat(szModel, szSetting);
   sprintf(szSetting, "%u %u %u %d\n", rc_params.hid_id, rc_params.uRCFlags, rc_params.rcChAssignmentThrotleReverse, rc_params.iRCTranslationType );
   strcat(szModel, szSetting);

   //----------------------------------------
   // Misc

   sprintf(szSetting, "misc: %u %u %u %d\n", uModelPersistentStatusFlags, uDeveloperFlags, enc_flags, (int)alarms_params.uAlarmMotorCurrentThreshold);
   strcat(szModel, szSetting);
   sprintf(szSetting, "stats: %u %u %u %u %u ", m_Stats.uTotalFlights, m_Stats.uCurrentOnTime, m_Stats.uCurrentFlightTime, m_Stats.uCurrentFlightDistance, m_Stats.uCurrentFlightTotalCurrent);
   strcat(szModel, szSetting);
   sprintf(szSetting, "%u %u %u %u %u\n", m_Stats.uCurrentTotalCurrent, m_Stats.uCurrentMaxAltitude, m_Stats.uCurrentMaxDistance, m_Stats.uCurrentMaxCurrent, m_Stats.uCurrentMinVoltage);
   strcat(szModel, szSetting);
   sprintf(szSetting, "%u %u %u ", m_Stats.uTotalOnTime, m_Stats.uTotalFlightTime, m_Stats.uTotalFlightDistance);
   strcat(szModel, szSetting);
   sprintf(szSetting, "%u %u %u %u %u\n", m_Stats.uTotalTotalCurrent, m_Stats.uTotalMaxAltitude, m_Stats.uTotalMaxDistance, m_Stats.uTotalMaxCurrent, m_Stats.uTotalMinVoltage);
   strcat(szModel, szSetting);

   //----------------------------------------
   // Functions triggers 

   sprintf(szSetting, "func: %d %d %d ", functions_params.bEnableRCTriggerFreqSwitchLink1, functions_params.bEnableRCTriggerFreqSwitchLink2, functions_params.bEnableRCTriggerFreqSwitchLink3);
   strcat(szModel, szSetting);
   sprintf(szSetting, "%d %d %d ", functions_params.iRCTriggerChannelFreqSwitchLink1, functions_params.iRCTriggerChannelFreqSwitchLink2, functions_params.iRCTriggerChannelFreqSwitchLink3);
   strcat(szModel, szSetting);
   sprintf(szSetting, "%d %d %d\n", functions_params.bRCTriggerFreqSwitchLink1_is3Position, functions_params.bRCTriggerFreqSwitchLink2_is3Position, functions_params.bRCTriggerFreqSwitchLink3_is3Position);
   strcat(szModel, szSetting);

   for( int i=0; i<3; i++ )
   {
      sprintf(szSetting, "%u %u %u %u %u %u\n", functions_params.uChannels433FreqSwitch[i], functions_params.uChannels868FreqSwitch[i], functions_params.uChannels23FreqSwitch[i], functions_params.uChannels24FreqSwitch[i], functions_params.uChannels25FreqSwitch[i], functions_params.uChannels58FreqSwitch[i]);
      strcat(szModel, szSetting);
   }
   for( unsigned int i=0; i<(sizeof(functions_params.uDummyF)/sizeof(functions_params.uDummyF[0])); i++ )
   {
      sprintf(szSetting, " %u",functions_params.uDummyF[i]);
      strcat(szModel, szSetting);
   }
   sprintf(szSetting, "\n");
   strcat(szModel, szSetting);

   //-------------------------------------------
   // End of standard params
   sprintf(szSetting, "END_ST\n");
   strcat(szModel, szSetting);

   //-------------------------------------------
   // Start of extra params, might be zero on load

   // End writing values to file
   // ---------------------------------------------------

   // Done
   fprintf(fd, "%s", szModel);
   return true;
}
