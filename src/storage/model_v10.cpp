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
#include "model_legacy_limits.h"

bool Model::loadVersion10(FILE* fd)
{
   char szBuff[256];
   int tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0, tmp5 = 0, tmp6 = 0, tmp7 = 0;
   u32 tmp32 = 0;
   u32 u1 = 0, u2 = 0, u3 = 0, u4 = 0, u5 = 0, u6 = 0, u7 = 0;
   int vt = 0;
   deprecated_type_radio_runtime_capabilities_parameters radioRuntimeCapabilities;
   bool bOk = true;

   if ( 1 != fscanf(fd, "%s", szBuff) )
      { log_softerror_and_alarm("10-1"); return false; }

   if ( 1 != fscanf(fd, "%*s %d", &iSaveCount) )
      { log_softerror_and_alarm("10-2"); return false; }

   if ( 4 != fscanf(fd, "%*s %u %d %d %u", &sw_version, &uVehicleId, &uControllerId, &hwCapabilities.uBoardType) )
      { log_softerror_and_alarm("10-3"); return false; }

   if ( hardware_is_vehicle() )
      sw_version = (SYSTEM_SW_VERSION_MAJOR * 256 + SYSTEM_SW_VERSION_MINOR) | (SYSTEM_SW_BUILD_NUMBER<<16);

   if ( bOk && (1 != fscanf(fd, "%u", &uModelFlags )) )
      { log_softerror_and_alarm("10-4"); uModelFlags = 0; return false; }


   if ( 1 != fscanf(fd, "%s", vehicle_name) )
      { log_softerror_and_alarm("10-5"); return false; }
   if ( vehicle_name[0] == '*' && vehicle_name[1] == 0 )
      vehicle_name[0] = 0;

   for( int i=0; i<(int)strlen(vehicle_name); i++ )
      if ( vehicle_name[i] == '_' )
         vehicle_name[i] = ' ';

   str_sanitize_modelname(vehicle_name);

   if ( 3 != fscanf(fd, "%d %u %d", &rxtx_sync_type, &camera_rc_channels, &tmp1 ) )
      { log_softerror_and_alarm("10-6"); return false; }

   if ( 4 != fscanf(fd, "%d %d %u %d", &tmp1, &vt, &m_Stats.uTotalFlightTime, &iGPSCount ) )
      { log_softerror_and_alarm("10-7"); return false; }

   is_spectator = (bool)tmp1;
   vehicle_type = vt;

   //----------------------------------------
   // CPU

   if ( 3 != fscanf(fd, "%*s %d %d %d", &tmp1, &tmp2, &processesPriorities.ioNiceVideo) )
      { log_softerror_and_alarm("10-8"); return false; }
   if ( 3 != fscanf(fd, "%d %d %d", &processesPriorities.iOverVoltage, &processesPriorities.iFreqARM, &processesPriorities.iFreqGPU) )
      { log_softerror_and_alarm("10-9"); }
   if ( 3 != fscanf(fd, "%d %d %d", &tmp1, &processesPriorities.ioNiceRouter, &tmp2) )
      { log_softerror_and_alarm("10-10"); }

   //----------------------------------------
   // Radio

   if ( 1 != fscanf(fd, "%*s %d", &radioInterfacesParams.interfaces_count) )
      { log_softerror_and_alarm("10-11"); return false; }

   for( int i=0; i<radioInterfacesParams.interfaces_count; i++ )
   {
      char szTmp[256];
      char szTmp2[256];
      if ( 3 != fscanf(fd, "%d %d %u", &(radioInterfacesParams.interface_card_model[i]), &(radioInterfacesParams.interface_link_id[i]), &(radioInterfacesParams.interface_current_frequency_khz[i])) )
         { log_softerror_and_alarm("10-12"); return false; }

      if ( 8 != fscanf(fd, "%u %d %u %d %d %d %s %s", &u4, &tmp2, &tmp32, &tmp5, &tmp6, &tmp7, szTmp2, szTmp) )
         { log_softerror_and_alarm("10-13"); return false; }
      radioInterfacesParams.interface_capabilities_flags[i] = u4;
      radioInterfacesParams.interface_supported_bands[i] = (u8)tmp2;
      radioInterfacesParams.interface_radiotype_and_driver[i] = tmp32;
      radioInterfacesParams.interface_supported_radio_flags[i] = tmp5;
      radioInterfacesParams.interface_raw_power[i] = tmp6;

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

   if ( 9 != fscanf(fd, "%d %d %d %u %d %d %d %d %d", &tmp1, &radioInterfacesParams.iAutoVehicleTxPower, &radioInterfacesParams.iAutoControllerTxPower, &radioInterfacesParams.uFlagsRadioInterfaces, &tmp2, &tmp3, &tmp4,  &tmp5, &tmp5) )
      { log_softerror_and_alarm("10-14"); return false; }
   enableDHCP = (bool)tmp1;

   fscanf(fd, "%d", &tmp1);

   if ( 1 != fscanf(fd, "%*s %d", &radioLinksParams.links_count) )
      { log_softerror_and_alarm("10-15"); return false; }

   for( int i=0; i<radioLinksParams.links_count; i++ )
   {
      if ( 5 != fscanf(fd, "%u %u %u %d %d", &(radioLinksParams.link_frequency_khz[i]), &(radioLinksParams.link_capabilities_flags[i]), &(radioLinksParams.link_radio_flags_tx[i]), &(radioLinksParams.downlink_datarate_video_bps[i]), &(radioLinksParams.downlink_datarate_data_bps[i])) )
         { log_softerror_and_alarm("10-16"); return false; }

      if ( 4 != fscanf(fd, "%d %u %d %d", &tmp1, &u1, &(radioLinksParams.uplink_datarate_video_bps[i]), &(radioLinksParams.uplink_datarate_data_bps[i])) )
         { log_softerror_and_alarm("10-17"); return false; }
      radioLinksParams.uSerialPacketSize[i] = tmp1;

      if ( 2 != fscanf(fd, "%d %d", &tmp1, &tmp2) )
      {
         log_softerror_and_alarm("10-18");
         radioLinksParams.uMaxLinkLoadPercent[i] = DEFAULT_RADIO_LINK_LOAD_PERCENT;
         tmp1 = 0;
         tmp2 = 0;
      }
      else
      {
         radioLinksParams.uMaxLinkLoadPercent[i] = tmp1;
      }
   }

   if ( 2 != fscanf(fd, "%d %u", &radioLinksParams.iSiKPacketSize, &radioLinksParams.uGlobalRadioLinksFlags) )
   {
      log_softerror_and_alarm("10-19");
      return false;
   }
   for( unsigned int j=0; j<7; j++ )
   {
      if ( 1 != fscanf(fd, "%d", &tmp1) )
         { log_softerror_and_alarm("10-20"); return false; }
   }

   //-------------------------------
   // Relay params

   if ( 5 != fscanf(fd, "%*s %d %u %d %u %u", &relay_params.isRelayEnabledOnRadioLinkId, &(relay_params.uRelayFrequencyKhz), &tmp2, &relay_params.uRelayedVehicleId, &relay_params.uRelayCapabilitiesFlags) )
      { log_softerror_and_alarm("10-21"); return false; }
   relay_params.uCurrentRelayMode = tmp2;

   //----------------------------------------
   // Telemetry

   if ( 5 != fscanf(fd, "%*s %d %d %d %d %d", &telemetry_params.fc_telemetry_type, &tmp3, &telemetry_params.iUpdateRateHz, &tmp1, &tmp2) )
      { log_softerror_and_alarm("10-22"); return false; }

   if ( 4 != fscanf(fd, "%d %u %d %u", &telemetry_params.iVideoBitrateHistoryGraphSampleInterval, &u1, &tmp1, &u2) )
      { log_softerror_and_alarm("10-23"); return false; }

   if ( 3 != fscanf(fd, "%d %d %d", &telemetry_params.vehicle_mavlink_id, &telemetry_params.controller_mavlink_id, &telemetry_params.flags) )
      { log_softerror_and_alarm("10-24"); return false; }

   fscanf(fd, "%d %u", &tmp1, &u1);

   if ( 1 != fscanf(fd, "%d", &tmp1) ) // not used
      { log_softerror_and_alarm("10-25"); return false; }

   //----------------------------------------
   // Video

   if ( 4 != fscanf(fd, "%*s %d %d %d %u", &video_params.iCurrentVideoProfile, &video_params.iH264Slices, &u2, &video_params.lowestAllowedAdaptiveVideoBitrate) )
      { log_softerror_and_alarm("10-26"); return false; }

   if ( 1 != fscanf(fd, "%u", &video_params.uMaxAutoKeyframeIntervalMs) )
      { log_softerror_and_alarm("10-27"); return false; }

   if ( 1 != fscanf(fd, "%u", &video_params.uVideoExtraFlags) )
      { log_softerror_and_alarm("10-28"); return false; }
   
   if ( 3 != fscanf(fd, "%d %d %d", &video_params.iVideoWidth, &video_params.iVideoHeight, &video_params.iVideoFPS) )
      { log_softerror_and_alarm("10-29"); return false; }

   if ( bOk && (1 != fscanf(fd, "%*s %d", &tmp7)) )
      { log_softerror_and_alarm("10-30"); bOk = false; tmp7=0; }

   for( int i=0; i<MAX_VIDEO_LINK_PROFILES; i++ )
      { video_link_profiles[i].uProfileFlags = 0; video_link_profiles[i].uProfileEncodingFlags = 0; }

   if ( tmp7 < 0 || tmp7 > MAX_VIDEO_LINK_PROFILES )
      tmp7 = MAX_VIDEO_LINK_PROFILES;

   for( int i=0; i<tmp7; i++ )
   {
      if ( ! bOk )
        { log_softerror_and_alarm("10-31"); bOk = false; }
 
      if ( bOk && (6 != fscanf(fd, "%u %u %u %d %u %d", &(video_link_profiles[i].uProfileFlags), &(video_link_profiles[i].uProfileEncodingFlags), &(video_link_profiles[i].uTargetVideoBitrateBPS), &(video_link_profiles[i].iAdaptiveAdjustmentStrength), &(video_link_profiles[i].uAdaptiveWeights), &(video_link_profiles[i].iDefaultFPS))) )
         { log_softerror_and_alarm("10-32"); bOk = false; }

      if ( bOk && (2 != fscanf(fd, "%d %d", &u2, &u3)) )
         { log_softerror_and_alarm("10-33"); bOk = false; }

      if ( bOk && (5 != fscanf(fd, "%d %d %d %d %d", &(video_link_profiles[i].iBlockDataPackets), &(video_link_profiles[i].iBlockECs), &(video_link_profiles[i].video_data_length), &tmp1, &(video_link_profiles[i].iKeyframeMS))) )
         { log_softerror_and_alarm("10-34"); bOk = false; }
      
      if ( bOk && (5 != fscanf(fd, "%d %d %d %d %d", &(video_link_profiles[i].h264profile), &(video_link_profiles[i].h264level), &(video_link_profiles[i].h264refresh), &(video_link_profiles[i].h264quantization), &(video_link_profiles[i].iIPQuantizationDelta))) )
         { log_softerror_and_alarm("10-35"); bOk = false; }
   }


   //----------------------------------------
   // Camera params

   if ( 2 != fscanf(fd, "%*s %d %d", &iCameraCount, &iCurrentCamera) )
      { log_softerror_and_alarm("10-36"); return false; }

   for( int k=0; k<MODEL_MAX_CAMERAS; k++ )
   {

      // Camera type:
      if ( 3 != fscanf(fd, "%*s %d %d %d", &(camera_params[k].iCameraType), &(camera_params[k].iForcedCameraType), &(camera_params[k].iCurrentProfile)) )
         { log_softerror_and_alarm("10-37"); return false; }

      //----------------------------------------
      // Camera sensor name

      char szTmp[1024];
      if ( bOk && (1 != fscanf(fd, "%*s %s", szTmp)) )
         { log_softerror_and_alarm("10-38"); camera_params[k].szCameraName[0] = 0; bOk = false; }
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
            { log_softerror_and_alarm("10-39 %d", i); return false; }
         if ( 1 != fscanf(fd, "%d", &tmp1) )
            { log_softerror_and_alarm("10-40 %d", i); return false; }
         camera_params[k].profiles[i].flip_image = (bool)tmp1;

         if ( 4 != fscanf(fd, "%d %d %d %d", &tmp1, &tmp2, &tmp3, &tmp4) )
            { log_softerror_and_alarm("10-41 %d", i); return false; }
         camera_params[k].profiles[i].brightness = tmp1;
         camera_params[k].profiles[i].contrast = tmp2;
         camera_params[k].profiles[i].saturation = tmp3;
         camera_params[k].profiles[i].sharpness = tmp4;

         if ( 4 != fscanf(fd, "%d %d %d %d", &tmp1, &tmp2, &tmp3, &tmp4) )
            { log_softerror_and_alarm("10-42 %d", i); return false; }
         camera_params[k].profiles[i].exposure = tmp1;
         camera_params[k].profiles[i].whitebalance = tmp2;
         camera_params[k].profiles[i].metering = tmp3;
         camera_params[k].profiles[i].drc = tmp4;

         if ( 3 != fscanf(fd, "%f %f %f", &(camera_params[k].profiles[i].analogGain), &(camera_params[k].profiles[i].awbGainB), &(camera_params[k].profiles[i].awbGainR)) )
            { log_softerror_and_alarm("10-43 %d", i); }

         if ( 2 != fscanf(fd, "%f %f", &(camera_params[k].profiles[i].fovH), &(camera_params[k].profiles[i].fovV)) )
            { log_softerror_and_alarm("10-44 %d", i); }

         if ( 4 != fscanf(fd, "%d %d %d %d", &tmp1, &tmp2, &tmp3, &tmp4) )
            { log_softerror_and_alarm("10-45 %d", i); return false; }
         camera_params[k].profiles[i].vstab = tmp1;
         camera_params[k].profiles[i].ev = tmp2;
         camera_params[k].profiles[i].iso = tmp3;
         camera_params[k].profiles[i].iShutterSpeed = tmp4;

         if ( 1 != fscanf(fd, "%d", &tmp1) )
            { log_softerror_and_alarm("10-46 %d", i); }
         else
            camera_params[k].profiles[i].wdr = (u8)tmp1;

         if ( 1 != fscanf(fd, "%d", &tmp1) )
            { log_softerror_and_alarm("10-48 %d", i); camera_params[k].profiles[i].dayNightMode = 0; }
         else
            camera_params[k].profiles[i].dayNightMode = (u8)tmp1;
           
         if ( 1 != fscanf(fd, "%d", &tmp1) )
            { log_softerror_and_alarm("10-49 %d", i); camera_params[k].profiles[i].hue = 0; }
         else
            camera_params[k].profiles[i].hue = (u8)tmp1;

         if ( 1 != fscanf(fd, "%d", &tmp1) )
            { log_softerror_and_alarm("10-50 %d", i); return false; }
         camera_params[k].profiles[i].uDummyCamP = tmp1;
      }

   }

   //----------------------------------------
   // Audio Settings

   audio_params.has_audio_device = false;
   if ( 1 == fscanf(fd, "%*s %d", &tmp1) )
      audio_params.has_audio_device = (bool)tmp1;
   else
      { bOk = false; log_softerror_and_alarm("10-51"); }
   if ( 4 == fscanf(fd, "%d %d %d %u", &tmp2, &audio_params.volume, &audio_params.quality, &audio_params.uFlags) )
   {
      audio_params.has_audio_device = tmp1;
      audio_params.enabled = tmp2;
   }
   else
   {
      bOk = false;
      log_softerror_and_alarm("10-52");
   }
   
   if ( 1 != fscanf(fd, "%*s %u", &alarms) )
      alarms = 0;

   //----------------------------------------
   // Hardware info

   if ( 4 == fscanf(fd, "%*s %d %d %d %d", &hardwareInterfacesInfo.radio_interface_count, &hardwareInterfacesInfo.i2c_bus_count, &hardwareInterfacesInfo.i2c_device_count, &hardwareInterfacesInfo.serial_port_count) )
   {
      if ( hardwareInterfacesInfo.i2c_bus_count < 0 || hardwareInterfacesInfo.i2c_bus_count > MAX_MODEL_I2C_BUSSES )
         { log_softerror_and_alarm("10-53"); return false; }
      if ( hardwareInterfacesInfo.i2c_device_count < 0 || hardwareInterfacesInfo.i2c_device_count > MAX_MODEL_I2C_DEVICES )
         { log_softerror_and_alarm("10-54"); return false; }
      if ( hardwareInterfacesInfo.serial_port_count < 0 || hardwareInterfacesInfo.serial_port_count > MAX_MODEL_SERIAL_PORTS )
         { log_softerror_and_alarm("10-55"); return false; }

      for( int i=0; i<hardwareInterfacesInfo.i2c_bus_count; i++ )
         if ( 1 != fscanf(fd, "%d", &(hardwareInterfacesInfo.i2c_bus_numbers[i])) )
            { log_softerror_and_alarm("10-56"); return false; }

      for( int i=0; i<hardwareInterfacesInfo.i2c_device_count; i++ )
         if ( 2 != fscanf(fd, "%d %d", &(hardwareInterfacesInfo.i2c_devices_bus[i]), &(hardwareInterfacesInfo.i2c_devices_address[i])) )
            { log_softerror_and_alarm("10-57"); return false; }

      for( int i=0; i<hardwareInterfacesInfo.serial_port_count; i++ )
         if ( 3 != fscanf(fd, "%d %u %s", &(hardwareInterfacesInfo.serial_port_speed[i]), &(hardwareInterfacesInfo.serial_port_supported_and_usage[i]), &(hardwareInterfacesInfo.serial_port_names[i][0])) )
            { log_softerror_and_alarm("10-58"); return false; }
   }
   else
   {
      hardwareInterfacesInfo.radio_interface_count = 0;
      hardwareInterfacesInfo.i2c_bus_count = 0;
      hardwareInterfacesInfo.i2c_device_count = 0;
      hardwareInterfacesInfo.serial_port_count = 0;
      bOk = false;
   }

   //----------------------------------------
   // OSD

   if ( 5 != fscanf(fd, "%*s %d %d %f %d %d", &osd_params.iCurrentOSDScreen, &tmp1, &osd_params.voltage_alarm, &tmp2, &tmp3) )
      { log_softerror_and_alarm("10-59"); return false; }
   osd_params.voltage_alarm_enabled = (bool)tmp1;
   osd_params.altitude_relative = (bool)tmp2;
   osd_params.show_gps_position = (bool)tmp3;

   if ( 5 != fscanf(fd, "%d %d %d %d %d", &osd_params.battery_show_per_cell,  &osd_params.battery_cell_count, &osd_params.battery_capacity_percent_alarm, &tmp1, &osd_params.home_arrow_rotate) )
      { log_softerror_and_alarm("10-60"); return false; }
   osd_params.invert_home_arrow = (bool)tmp1;

   if ( 7 != fscanf(fd, "%d %d %d %d %d %d %d", &tmp1, &tmp2, &tmp3, &tmp4, &tmp5, &tmp6, &osd_params.ahi_warning_angle) )
      { log_softerror_and_alarm("10-61"); return false; }

   osd_params.show_overload_alarm = (bool)tmp1;
   osd_params.show_stats_rx_detailed = (bool)tmp2;
   osd_params.show_stats_decode = (bool)tmp3;
   osd_params.show_stats_rc = (bool)tmp4;
   osd_params.show_full_stats = (bool)tmp5;
   osd_params.show_instruments = (bool)tmp6;

   for( int i=0; i<MODEL_MAX_OSD_SCREENS; i++ )
      if ( 5 != fscanf(fd, "%u %u %u %u %u", &(osd_params.osd_flags[i]), &(osd_params.osd_flags2[i]), &(osd_params.osd_flags3[i]), &(osd_params.instruments_flags[i]), &(osd_params.osd_preferences[i])) )
         { bOk = false; log_softerror_and_alarm("10-62"); }

   //----------------------------------------
   // RC

   if ( 4 != fscanf(fd, "%*s %d %d %d %d", &tmp1, &tmp2, &rc_params.receiver_type, &rc_params.rc_frames_per_second ) )
      { log_softerror_and_alarm("10-63"); return false; }

   if ( 1 != fscanf(fd, "%d", &rc_params.inputType) )
      { log_softerror_and_alarm("10-64"); return false; }

   if ( 4 != fscanf(fd, "%d %d %d %d", &tmp1, &tmp2, &tmp3, &tmp4 ) )
      { log_softerror_and_alarm("10-65"); return false; }
   
   for( int i=0; i<MAX_RC_CHANNELS; i++ )
   {
      if ( 7 != fscanf(fd, "%u %u %u %u %u %u %u", &u1, &u2, &u3, &u4, &u5, &u6, &u7) )
         { log_softerror_and_alarm("10-66"); return false; }
      rc_params.rcChAssignment[i] = u1;
      rc_params.rcChMid[i] = u2;
      rc_params.rcChMin[i] = u3;
      rc_params.rcChMax[i] = u4;
      rc_params.rcChFailSafe[i] = u5;
      rc_params.rcChExpo[i] = u6;
      rc_params.rcChFlags[i] = u7;
   }
   
   if ( 3 != fscanf(fd, "%d %u %u", &rc_params.rc_failsafe_timeout_ms, &rc_params.failsafeFlags, &rc_params.channelsCount ) )
      { log_softerror_and_alarm("10-67"); return false; }
   if ( 1 != fscanf(fd, "%u", &rc_params.hid_id ) )
      { log_softerror_and_alarm("10-68"); return false; }

   if ( 1 != fscanf(fd, "%u", &rc_params.uRCFlags ) )
      { log_softerror_and_alarm("10-69"); return false; }

   if ( 1 != fscanf(fd, "%u", &rc_params.rcChAssignmentThrotleReverse ) )
      { log_softerror_and_alarm("10-70"); return false; }

   if ( 1 != fscanf(fd, "%d", &rc_params.iRCTranslationType ) )
      { log_softerror_and_alarm("10-71"); return false; }

   for( unsigned int i=0; i<8; i++ )
      if ( 1 != fscanf(fd, "%u", &u1) )
         { log_softerror_and_alarm("10-72"); return false; }

   //----------------------------------------
   // Misc

   if ( 2 != fscanf(fd, "%*s %u %u", &uModelPersistentStatusFlags, &uDeveloperFlags) )
      { log_softerror_and_alarm("10-73"); uDeveloperFlags = (((u32)DEFAULT_DELAY_WIFI_CHANGE)<<DEVELOPER_FLAGS_WIFI_GUARD_DELAY_MASK_SHIFT); }
  
   if ( 1 != fscanf(fd, "%u", &enc_flags) )
   {
      enc_flags = MODEL_ENC_FLAGS_NONE;
      log_softerror_and_alarm("10-74");
   }

   if ( bOk && (1 != fscanf(fd, "%*s %u", &m_Stats.uTotalFlights)) )
   {
      log_softerror_and_alarm("10-75");
      m_Stats.uTotalFlights = 0;
      bOk = false;
   }
   if ( bOk && (4 != fscanf(fd, "%u %u %u %u", &m_Stats.uCurrentOnTime, &m_Stats.uCurrentFlightTime, &m_Stats.uCurrentFlightDistance, &m_Stats.uCurrentFlightTotalCurrent)) )
   {
      log_softerror_and_alarm("10-76");
      m_Stats.uCurrentOnTime = 0; m_Stats.uCurrentFlightTime = 0; m_Stats.uCurrentFlightDistance = 0;
      bOk = false;
   }
   if ( bOk && (5 != fscanf(fd, "%u %u %u %u %u", &m_Stats.uCurrentTotalCurrent, &m_Stats.uCurrentMaxAltitude, &m_Stats.uCurrentMaxDistance, &m_Stats.uCurrentMaxCurrent, &m_Stats.uCurrentMinVoltage)) )
   {
      log_softerror_and_alarm("10-77");
      m_Stats.uCurrentTotalCurrent = 0;
      m_Stats.uCurrentMaxAltitude = 0;
      m_Stats.uCurrentMaxDistance = 0;
      m_Stats.uCurrentMaxCurrent = 0;
      m_Stats.uCurrentMinVoltage = 100000;
      bOk = false;
   }

   if ( bOk && (3 != fscanf(fd, "%u %u %u", &m_Stats.uTotalOnTime, &m_Stats.uTotalFlightTime, &m_Stats.uTotalFlightDistance)) )
   {
      log_softerror_and_alarm("10-78");
      m_Stats.uTotalOnTime = 0; m_Stats.uTotalFlightTime = 0; m_Stats.uTotalFlightDistance = 0;
      bOk = false;
   }
   if ( bOk && (5 != fscanf(fd, "%u %u %u %u %u", &m_Stats.uTotalTotalCurrent, &m_Stats.uTotalMaxAltitude, &m_Stats.uTotalMaxDistance, &m_Stats.uTotalMaxCurrent, &m_Stats.uTotalMinVoltage)) )
   {
      log_softerror_and_alarm("10-79");
      m_Stats.uTotalTotalCurrent = 0;
      m_Stats.uTotalMaxAltitude = 0;
      m_Stats.uTotalMaxDistance = 0;
      m_Stats.uTotalMaxCurrent = 0;
      m_Stats.uTotalMinVoltage = 100000;
      bOk = false;
   }


   //----------------------------------------
   // Functions & Triggers

   if ( bOk && (3 != fscanf(fd, "%*s %d %d %d", &tmp1, &tmp2, &tmp3 )) )
      { log_softerror_and_alarm("10-80"); bOk = false; tmp1 = 0; tmp2 = 0; tmp3 = 0; }

   functions_params.bEnableRCTriggerFreqSwitchLink1 = (bool)tmp1;
   functions_params.bEnableRCTriggerFreqSwitchLink2 = (bool)tmp2;
   functions_params.bEnableRCTriggerFreqSwitchLink3 = (bool)tmp3;


   if ( bOk && (3 != fscanf(fd, "%d %d %d", &functions_params.iRCTriggerChannelFreqSwitchLink1, &functions_params.iRCTriggerChannelFreqSwitchLink2, &functions_params.iRCTriggerChannelFreqSwitchLink3 )) )
      { log_softerror_and_alarm("10-81"); bOk = false; functions_params.iRCTriggerChannelFreqSwitchLink1 = -1; functions_params.iRCTriggerChannelFreqSwitchLink2 = -1; functions_params.iRCTriggerChannelFreqSwitchLink3 = -1; }

   if ( bOk && (3 != fscanf(fd, "%d %d %d", &tmp1, &tmp2, &tmp3 )) )
      { log_softerror_and_alarm("10-82"); bOk = false; }

   functions_params.bRCTriggerFreqSwitchLink1_is3Position = (bool)tmp1;
   functions_params.bRCTriggerFreqSwitchLink2_is3Position = (bool)tmp2;
   functions_params.bRCTriggerFreqSwitchLink3_is3Position = (bool)tmp3;

   for( int i=0; i<3; i++ )
   {
      if ( bOk && (6 != fscanf(fd, "%u %u %u %u %u %u", &functions_params.uChannels433FreqSwitch[i], &functions_params.uChannels868FreqSwitch[i], &functions_params.uChannels23FreqSwitch[i], &functions_params.uChannels24FreqSwitch[i], &functions_params.uChannels25FreqSwitch[i], &functions_params.uChannels58FreqSwitch[i])) )
         { log_softerror_and_alarm("10-83"); bOk = false; }
   }

   for( unsigned int i=0; i<12; i++ )
      if ( bOk && (1 != fscanf(fd, "%u", &u1)) )
         { log_softerror_and_alarm("10-84"); bOk = false; }

   //----------------------------------------------------
   // Start of extra params, might be zero on load

   if ( 1 != fscanf(fd, "%d", &tmp1) )
      alarms_params.uAlarmMotorCurrentThreshold = (1<<7) & 30;
   else
      alarms_params.uAlarmMotorCurrentThreshold = tmp1;

   if ( 1 != fscanf(fd, "%d", &osd_params.iRadioInterfacesGraphRefreshIntervalMs) )
      osd_params.iRadioInterfacesGraphRefreshIntervalMs = DEFAULT_OSD_RADIO_GRAPH_REFRESH_PERIOD_MS;

   if ( 3 != fscanf(fd, "%d %d %u", &hwCapabilities.iMaxTxVideoBlocksBuffer, &hwCapabilities.iMaxTxVideoBlockPackets, &hwCapabilities.uHWFlags) )
      resetHWCapabilities();

   if ( 1 != fscanf(fd, "%u", &hwCapabilities.uRubyBaseVersion) )
      hwCapabilities.uRubyBaseVersion = 0;
   for( unsigned int i=0; i<1; i++ )
      if ( bOk && (1 != fscanf(fd, "%d", &tmp1)) )
         { bOk = false; }

   for( unsigned int i=0; i<3; i++ )
      if ( bOk && (1 != fscanf(fd, "%u", &u1)) )
         { bOk = false; }

   if ( ! bOk )
      log_softerror_and_alarm("10-85");
   if ( bOk )
   if ( 3 != fscanf(fd, "%d %d %d", &processesPriorities.iThreadPriorityRadioRx, &processesPriorities.iThreadPriorityRadioTx, &processesPriorities.iThreadPriorityRouter) )
   {
      log_softerror_and_alarm("10-86");
      resetProcessesParams();
   }

   if ( bOk && (1 != fscanf(fd, "%u", &osd_params.uFlags)) )
   {
      log_softerror_and_alarm("10-87");
      osd_params.uFlags = 0;
      bOk = false;
   }

   if ( bOk && (1 != fscanf(fd, "%u", &processesPriorities.uProcessesFlags)) )
   {
      log_softerror_and_alarm("10-88");
      resetProcessesParams();
      bOk = false;
   }

   if ( bOk && (3 != fscanf(fd, "%d %d %d", &video_params.iRemovePPSVideoFrames, &video_params.iInsertPPSVideoFrames, &video_params.iInsertSPTVideoFramesTimings)) )
   {
      log_softerror_and_alarm("10-89");
      video_params.iRemovePPSVideoFrames = 0;
      video_params.iInsertPPSVideoFrames = 1;
      video_params.iInsertSPTVideoFramesTimings = 0;
      bOk = false;
   }

   if ( bOk && (3 != fscanf(fd, "%d %d %u", &tmp1, &tmp2, &audio_params.uDummyA1)) )
   {
      log_softerror_and_alarm("10-90");
      audio_params.uECScheme = (((u32)DEFAULT_AUDIO_P_DATA) << 4) | ((u32)DEFAULT_AUDIO_P_EC);
      audio_params.uDummyA1 = 0;
      audio_params.uPacketLength = DEFAULT_AUDIO_PACKET_LENGTH;
   }
   audio_params.uECScheme = (u8)tmp1;
   audio_params.uPacketLength = (u16)tmp2;

   if ( bOk )
   {
      for( int i=0; i<MODEL_MAX_OSD_SCREENS; i++ )
      {
         if ( 1 != fscanf(fd, "%d", &tmp1) )
            osd_params.osd_layout_preset[i] = OSD_PRESET_DEFAULT;
         else
            osd_params.osd_layout_preset[i] = (u8)tmp1;
      }
   }

   if ( bOk )
   {
      for( int i=0; i<MAX_VIDEO_LINK_PROFILES; i++ )
      {
         if ( 1 != fscanf(fd, "%d", &video_link_profiles[i].iECPercentage) )
            video_link_profiles[i].iECPercentage = DEFAULT_VIDEO_EC_RATE_HQ;
      }
   }

   for( int i=0; i<MODEL_MAX_CAMERAS; i++ )
   {
      camera_params[i].iCameraBinProfile = 0;
      camera_params[i].szCameraBinProfileName[0] = 0;
   }

   if ( bOk )
   {
      for( int i=0; i<MODEL_MAX_CAMERAS; i++ )
      {
         char szTmpBin[MAX_CAMERA_BIN_PROFILE_NAME];
         if ( 2 != fscanf(fd, "%d %s", &camera_params[i].iCameraBinProfile, szTmpBin) )
         {
            camera_params[i].iCameraBinProfile = 0;
            camera_params[i].szCameraBinProfileName[0] = 0;
         }
         else
         {
            strncpy(camera_params[i].szCameraBinProfileName, szTmpBin, MAX_CAMERA_BIN_PROFILE_NAME-1);
            camera_params[i].szCameraBinProfileName[MAX_CAMERA_BIN_PROFILE_NAME-1] = 0;
            if ( '-' == camera_params[i].szCameraBinProfileName[0] )
               camera_params[i].szCameraBinProfileName[0] = 0;
         }
      }
   }

   if ( bOk )
   {
      tmp1 = 0;
      if ( 4 != fscanf(fd, "%d %d %d %u", &tmp1, &radioRuntimeCapabilities.iMaxSupportedMCSDataRate, &radioRuntimeCapabilities.iMaxSupportedLegacyDataRate, &radioRuntimeCapabilities.uSupportedMCSFlags) )
         resetRadioInterfacesRuntimeCapabilities(&radioInterfacesRuntimeCapab);
      else
      {
         radioRuntimeCapabilities.uFlagsRuntimeCapab = (u8)tmp1;
         bool bCapabOk = true;
         for( int iLink=0; iLink<DEP_MODEL_MAX_STORED_QUALITIES_LINKS; iLink++ )
         {
            for( int i=0; i<DEP_MODEL_MAX_STORED_QUALITIES_VALUES; i++ )
            {
               if ( 1 != fscanf(fd, "%f", &radioRuntimeCapabilities.fQualitiesLegacy[iLink][i]) )
               {
                  bCapabOk = false;
                  break;
               }
            }
            for( int i=0; i<DEP_MODEL_MAX_STORED_QUALITIES_VALUES; i++ )
            {
               if ( 1 != fscanf(fd, "%f", &radioRuntimeCapabilities.fQualitiesMCS[iLink][i]) )
               {
                  bCapabOk = false;
                  break;
               }
            }
            if ( ! bCapabOk )
               break;
         }

         for( int iLink=0; iLink<DEP_MODEL_MAX_STORED_QUALITIES_LINKS; iLink++ )
         {
            for( int i=0; i<DEP_MODEL_MAX_STORED_QUALITIES_VALUES; i++ )
            {
               if ( 1 != fscanf(fd, "%d", &radioRuntimeCapabilities.iMaxTxPowerMwLegacy[iLink][i]) )
               {
                  bCapabOk = false;
                  break;
               }
            }
            for( int i=0; i<DEP_MODEL_MAX_STORED_QUALITIES_VALUES; i++ )
            {
               if ( 1 != fscanf(fd, "%d", &radioRuntimeCapabilities.iMaxTxPowerMwMCS[iLink][i]) )
               {
                  bCapabOk = false;
                  break;
               }
            }
            if ( ! bCapabOk )
               break;
         }

         if ( ! bCapabOk )
            resetRadioInterfacesRuntimeCapabilities(&radioInterfacesRuntimeCapab);
      }
   }
   else
      resetRadioInterfacesRuntimeCapabilities(&radioInterfacesRuntimeCapab);

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

   for( int i=0; i<MAX_VIDEO_LINK_PROFILES; i++ )
   {
      video_link_profiles[i].iDefaultFPS = 0;
      video_link_profiles[i].iDefaultLinkLoad = 0;
      video_link_profiles[i].uDummyVP1 = 0;
      video_link_profiles[i].uDummyVP2 = 0;
   }

   resetProcessesParams();
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
