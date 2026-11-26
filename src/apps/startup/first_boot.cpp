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

#include "../../core/base.h"
#include "../../core/config.h"
#include "../../settings/models.h"
#include "../../platform/radxa_zero3w/hardware.h"
#include "../../platform/radxa_zero3w/hardware_files.h"
#include "../../platform/radxa_zero3w/hardware_radio.h"
#include "../../platform/radxa_zero3w/hardware_procs.h"
#include "../../settings/vehicle_settings.h"
#include "../../radio/link/radioflags.h"
#include "../../settings/ctrl_settings.h"
#include "first_boot.h"
#include "../../settings/ctrl_preferences.h"

Model s_ModelFirstBoot;

void do_first_boot_pre_initialization(bool bIgnoreDrivers)
{
   log_line("---------------------------------------");
   log_line("Do first time boot preinitialization...");

   char szOutput[9096];
   hw_execute_bash_command("lsmod", szOutput);
   log_line("Output of lsmod: [%s]", szOutput);
   log_line("------------------------------");

   printf("\nRuby: Doing first time ever initialization on Radxa. Please wait...\n");
   fflush(stdout);

   if ( ! bIgnoreDrivers )
      hardware_install_drivers(1);
   hw_execute_bash_command_raw("echo 'performance' | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor", NULL);

   hw_execute_bash_command_silent("mkdir -p /tmp/ruby/", NULL);
   log_init_local_only("RubyStartFirst");
   //log_enable_stdout();
   log_add_file("/tmp/ruby/log_first_radxa.log");
   hw_execute_bash_command("echo \"\nRuby: Doing first time ever initialization on Radxa...\n\" > /tmp/ruby/log_first_radxa.log", NULL);

   char szComm[256];
   sprintf(szComm, "mkdir -p %s", FOLDER_CONFIG);
   hw_execute_bash_command(szComm, NULL);

   hardware_set_default_radxa_cpu_freq();
   
   hw_execute_bash_command("sync", NULL);
   

   printf("\nRuby: Done doing first time ever initialization on Radxa.\n");
   fflush(stdout);
   hw_execute_bash_command("echo \"\nRuby: Done doing first time ever initialization on Radxa.\n\" > /tmp/ruby/log_first_radxa.log", NULL);

   log_line("Done first time boot preinitialization.");
   log_line("---------------------------------------");
}





void do_first_boot_initialization()
{
   log_line("-------------------------------------------------------");
   log_line("First Boot detected. Doing first boot initialization..." );

   char szComm[256];

   snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "mkdir -p %sbin_org", FOLDER_BINARIES);
   hw_execute_bash_command(szComm, NULL);
   snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "cp -rf %sruby_* %sbin_org/", FOLDER_BINARIES, FOLDER_BINARIES);
   hw_execute_bash_command(szComm, NULL);

   snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "chmod 777 %s*", FOLDER_CONFIG);
   hw_execute_bash_command(szComm, NULL);


   first_boot_create_default_model();

   {
      load_ControllerSettings();


      ControllerSettings* pcs = get_ControllerSettings();
      if ( NULL != pcs )
         pcs->iFreqARM = hardware_get_cpu_speed();
      save_ControllerSettings();
   }

   snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "rm -rf %s%s", FOLDER_CONFIG, FILE_CONFIG_FIRST_BOOT);
   hw_execute_bash_command_silent(szComm, NULL);
   hardware_sleep_ms(50);


   for( int i=0; i<hardware_get_radio_interfaces_count(); i++ )
   {
      if ( ! hardware_radio_index_is_wifi_radio(i) )
         continue;
      if ( hardware_radio_index_is_sik_radio(i) )
         continue;
      radio_hw_info_t* pRadioHWInfo = hardware_get_radio_info(i);
      if ( ! pRadioHWInfo->isConfigurable )
         continue;

      if ( hardware_radio_driver_is_rtl8812au_card(pRadioHWInfo->iRadioDriver) )
         hardware_radio_set_txpower_raw_rtl8812au(i, 10);
      if ( hardware_radio_driver_is_rtl8812eu_card(pRadioHWInfo->iRadioDriver) )
         hardware_radio_set_txpower_raw_rtl8812eu(i, 10);
      if ( hardware_radio_driver_is_atheros_card(pRadioHWInfo->iRadioDriver) )
         hardware_radio_set_txpower_raw_atheros(i, 10);
   }

   snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "chmod 777 %s*", FOLDER_CONFIG);
   hw_execute_bash_command(szComm, NULL);
   log_line("First boot initialization completed.");
   log_line("---------------------------------------------------------");
}


Model* first_boot_create_default_model()
{
   log_line("Creating a default model.");
   s_ModelFirstBoot.resetToDefaults(true);

   char szFile[MAX_FILE_PATH_SIZE];
   
   {
      s_ModelFirstBoot.resetRadioLinksParams();

      s_ModelFirstBoot.radioInterfacesParams.interfaces_count = 1;
      s_ModelFirstBoot.radioInterfacesParams.interface_radiotype_and_driver[0] = 0xFF0000;
      s_ModelFirstBoot.radioInterfacesParams.interface_supported_bands[0] = RADIO_HW_SUPPORTED_BAND_24;
      strcpy(s_ModelFirstBoot.radioInterfacesParams.interface_szMAC[0], "YYYYYY");
      strcpy(s_ModelFirstBoot.radioInterfacesParams.interface_szPort[0], "Y");
      s_ModelFirstBoot.radioInterfacesParams.interface_current_frequency_khz[0] = DEFAULT_FREQUENCY;
      s_ModelFirstBoot.radioInterfacesParams.interface_capabilities_flags[0] = RADIO_HW_CAPABILITY_FLAG_CAN_RX | RADIO_HW_CAPABILITY_FLAG_CAN_TX;
      s_ModelFirstBoot.radioInterfacesParams.interface_capabilities_flags[0] |= RADIO_HW_CAPABILITY_FLAG_CAN_USE_FOR_VIDEO | RADIO_HW_CAPABILITY_FLAG_CAN_USE_FOR_DATA;

      s_ModelFirstBoot.radioLinksParams.link_radio_flags_tx[0] = DEFAULT_RADIO_FRAMES_FLAGS;
      s_ModelFirstBoot.radioLinksParams.link_radio_flags_rx[0] = DEFAULT_RADIO_FRAMES_FLAGS;
      s_ModelFirstBoot.radioLinksParams.downlink_datarate_video_bps[0] = DEFAULT_RADIO_DATARATE_VIDEO;
      s_ModelFirstBoot.radioLinksParams.downlink_datarate_data_bps[0] = DEFAULT_RADIO_DATARATE_DATA;
      s_ModelFirstBoot.radioLinksParams.uplink_datarate_video_bps[0] = DEFAULT_RADIO_DATARATE_VIDEO;
      s_ModelFirstBoot.radioLinksParams.uplink_datarate_data_bps[0] = DEFAULT_RADIO_DATARATE_DATA;

      s_ModelFirstBoot.populateRadioInterfacesInfoFromHardware();

      s_ModelFirstBoot.b_mustSyncFromVehicle = true;
      s_ModelFirstBoot.is_spectator = false;

      strcpy(szFile, FOLDER_CONFIG);
      strcat(szFile, FILE_CONFIG_CURRENT_VEHICLE_MODEL);
      s_ModelFirstBoot.saveToFile(szFile, false);
   }

   return &s_ModelFirstBoot;
}