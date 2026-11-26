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
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "../../core/base.h"
#include "../../core/config.h"
#include "../../ipc/shared_mem.h"
#include "../../settings/models.h"
#include "../../settings/models_list.h"
#include "../../platform/radxa_zero3w/hardware.h"
#include "../../platform/radxa_zero3w/hardware_files.h"
#include "../../platform/radxa_zero3w/hardware_procs.h"
#include "../../platform/radxa_zero3w/hardware_radio_serial.h"
#include "../../settings/vehicle_settings.h"
#include "../../radio/link/radioflags.h"
#include "../../ipc/ruby_ipc.h"
#include "../../radio/tx_powers.h"

#include "../../settings/ctrl_settings.h"
#include "../../settings/utils_controller.h"
#include "../../settings/ctrl_interfaces.h"

#include "../../core/string_utils.h"
#include "../../core/i18n/strings_loc.h"
#include "r_initradio.h"
#include "first_boot.h"
#include "r_test.h"

static sem_t* s_pSemaphoreStarted = NULL; 

static int s_iBootCount = 0;

static bool g_bDebug = false;
static bool s_managedService = false;
static bool g_bIsFirstBoot = false;
static bool s_bIgnoreDrivers = false;

bool s_bQuit = false;
Model modelVehicle;

int _get_fast_reboot_counter()
{
   char szFile[MAX_FILE_PATH_SIZE];
   strcpy(szFile, FOLDER_CONFIG);
   strcat(szFile, FILE_CONFIG_FAST_BOOT_COUNTER);
   FILE* fd = fopen(szFile, "r");
   if ( NULL == fd )
      return 0;
   int iCounter = 0;
   if ( 1 != fscanf(fd, "%d", &iCounter) )
      iCounter = 0;
   fclose(fd);
   return iCounter;
}

void _increase_fast_reboot_counter()
{
   int iCounter = _get_fast_reboot_counter();
   iCounter++;
   char szFile[MAX_FILE_PATH_SIZE];
   strcpy(szFile, FOLDER_CONFIG);
   strcat(szFile, FILE_CONFIG_FAST_BOOT_COUNTER);
   hardware_file_check_and_fix_access(szFile);
   FILE* fd = fopen(szFile, "wt");
   if ( NULL == fd )
   {
      printf("\nERROR: Failed to write fast reboot counter %d\n", iCounter);
      return;
   }
   fprintf(fd, "%d\n", iCounter);
   fclose(fd);
   hardware_file_check_and_fix_access(szFile);
   printf("\nFast reboot counter %d -> %d\n", iCounter-1, _get_fast_reboot_counter());
}

void _reset_fast_reboot_counter()
{
   char szFile[MAX_FILE_PATH_SIZE];
   strcpy(szFile, FOLDER_CONFIG);
   strcat(szFile, FILE_CONFIG_FAST_BOOT_COUNTER);
   hardware_file_check_and_fix_access(szFile);
   FILE* fd = fopen(szFile, "wt");
   if ( NULL == fd )
      return;
   fprintf(fd, "%d\n", 0);
   fclose(fd);
   hardware_file_check_and_fix_access(szFile);
   printf("\nDid reset fast boot counter.\n");
}

void initLogFiles()
{
   char szComm[256];
   char szSrcFile[MAX_FILE_PATH_SIZE];
   snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "rm -rf %s%s", FOLDER_LOGS, LOG_FILE_LOGGER);
   hw_execute_bash_command_silent(szComm, NULL);

   strcpy(szSrcFile, FOLDER_LOGS);
   strcat(szSrcFile, LOG_FILE_SYSTEM);
   if( access( szSrcFile, R_OK ) != -1 )
   {
      sprintf(szComm, "mv %s %s/log_system_%d.txt", szSrcFile, FOLDER_LOGS, s_iBootCount-1 );
      hw_execute_bash_command_silent(szComm, NULL);
      sprintf(szComm, "touch %s", szSrcFile);
      hw_execute_bash_command_silent(szComm, NULL);
   }

   strcpy(szSrcFile, FOLDER_LOGS);
   strcat(szSrcFile, LOG_FILE_ERRORS);
   if( access( szSrcFile, R_OK ) != -1 )
   {
      sprintf(szComm, "mv %s %s/log_errors_%d.txt", szSrcFile, FOLDER_LOGS, s_iBootCount-1 );
      hw_execute_bash_command_silent(szComm, NULL);
      sprintf(szComm, "touch %s", szSrcFile);
      hw_execute_bash_command_silent(szComm, NULL);
   }

   strcpy(szSrcFile, FOLDER_LOGS);
   strcat(szSrcFile, LOG_FILE_ERRORS_SOFT);
   if( access( szSrcFile, R_OK ) != -1 )
   {
      sprintf(szComm, "mv %s %s/log_errors_soft_%d.txt", szSrcFile, FOLDER_LOGS, s_iBootCount-1 );
      hw_execute_bash_command_silent(szComm, NULL);
      sprintf(szComm, "touch %s", szSrcFile);
      hw_execute_bash_command_silent(szComm, NULL);
   }

   strcpy(szSrcFile, FOLDER_LOGS);
   strcat(szSrcFile, LOG_FILE_COMMANDS);
   if( access( szSrcFile, R_OK ) != -1 )
   {
      sprintf(szComm, "mv %s %s/log_commands_%d.txt", szSrcFile, FOLDER_LOGS, s_iBootCount-1 );
      hw_execute_bash_command_silent(szComm, NULL);
      sprintf(szComm, "touch %s", szSrcFile);
      hw_execute_bash_command_silent(szComm, NULL);
   }

   strcpy(szSrcFile, FOLDER_LOGS);
   strcat(szSrcFile, LOG_FILE_WATCHDOG);
   if( access( szSrcFile, R_OK ) != -1 )
   {
      sprintf(szComm, "mv %s %s/log_watchdog_%d.txt", szSrcFile, FOLDER_LOGS, s_iBootCount-1 );
      hw_execute_bash_command_silent(szComm, NULL);
      sprintf(szComm, "touch %s", szSrcFile);
      hw_execute_bash_command_silent(szComm, NULL);
   }

   strcpy(szSrcFile, FOLDER_LOGS);
   strcat(szSrcFile, LOG_FILE_VIDEO);
   if( access( szSrcFile, R_OK ) != -1 )
   {
      sprintf(szComm, "mv %s %s/log_video_%d.txt", szSrcFile, FOLDER_LOGS, s_iBootCount-1 );
      hw_execute_bash_command_silent(szComm, NULL);
   }

   if ( s_iBootCount >= 40 )
   {
      for( int i=0; i<=5; i++ )
      {
         sprintf(szComm, "rm -rf %s/logs*%d.txt 2>&1", FOLDER_LOGS, s_iBootCount-35-i);
         hw_execute_bash_command_silent(szComm, NULL);
      }
   }

   snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "rm -rf %s%s", FOLDER_LOGS, LOG_FILE_LIVE_VEHICLE_LOG);
   hw_execute_bash_command_silent(szComm, NULL);
}


void detectSystemType()
{
   log_line("Radxa Zero 3W ground station.");
   FILE* fd = fopen("/config/ruby_systype.txt", "w");
   if ( NULL != fd )
   {
      fprintf(fd, "STATION");
      fclose(fd);
   }
}

void _check_files()
{
   char szFilesMissing[1024];
   szFilesMissing[0] = 0;
   bool failed = false;

   if( access( "ruby_controller", R_OK ) == -1 )
      { failed = true; strcat(szFilesMissing, " ruby_controller"); }
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

   if( access( "ruby_central", R_OK ) == -1 )
      { failed = true; strcat(szFilesMissing, " ruby_central"); }
   if( access( "ruby_tx_rc", R_OK ) == -1 )
      { failed = true; strcat(szFilesMissing, " ruby_tx_rc"); }


   if ( failed )
      printf("Ruby: Checked files consistency: failed.\n");
   else
      printf("Ruby: Checked files consistency: ok.\n");
}

void _check_update_drivers_on_update()
{
   if ( s_bIgnoreDrivers )
      return;
   char szOutput[4098];
   bool bNeedsInstall = false;
   hw_execute_bash_command("lsmod | grep 88XXau", szOutput);
   if ( strlen(szOutput) < 6 )
   {
      if ( ! hardware_load_driver_rtl8812au() )
         bNeedsInstall = true;
   }
   hw_execute_bash_command("lsmod | grep 8812eu", szOutput);
   if ( strlen(szOutput) < 6 )
   {
      if ( ! hardware_load_driver_rtl8812eu() )
         bNeedsInstall = true;
   }
   if ( bNeedsInstall )
   {
      log_line("Updating drivers. Please wait...");
      printf("Ruby: Updating drivers. Please wait...\n");
      fflush(stdout);
      hardware_install_drivers(1);
   }
}

bool _init_timestamp_and_boot_count()
{
   bool bFirstBoot = false;
   s_iBootCount = 0;

   char szFile[128];
   strcpy(szFile, FOLDER_CONFIG);
   strcat(szFile, FILE_CONFIG_BOOT_COUNT);
   FILE* fd = fopen(szFile, "r");
   if ( NULL != fd )
   {
      if ( 1 != fscanf(fd, "%d", &s_iBootCount) )
      {
         log_softerror_and_alarm("Failed to read boot count from file %s", szFile);
         s_iBootCount = 0;
         bFirstBoot = true;
      }
      fclose(fd);
   }
   else
   {
      log_softerror_and_alarm("Failed to access boot count  file %s", szFile);
      bFirstBoot = true;
   }
   static long long lStartTimeStamp_ms;
   struct timespec t;
   clock_gettime(RUBY_HW_CLOCK_ID, &t);
   lStartTimeStamp_ms = t.tv_sec*1000LL + t.tv_nsec/1000LL/1000LL;
   
   strcpy(szFile, FOLDER_CONFIG);
   strcat(szFile, FILE_CONFIG_BOOT_TIMESTAMP);

   int count = 0;
   while ( count < 10 )
   {
      fd = fopen(szFile, "w");
      if ( NULL != fd )
      {
         fprintf(fd, "%lld\n", lStartTimeStamp_ms);
         fclose(fd);
         break;
      }
      count++;
   }
   s_iBootCount++;
   return bFirstBoot;
}

void _test_log(int argc, char *argv[])
{
   log_enable_stdout();
   log_init_local_only("RubyStart");
   log_arguments(argc, argv);

   int iKeyId = LOGGER_MESSAGE_QUEUE_ID;
   if ( argc >= 2 )
   if ( strcmp(argv[argc-2], "-testlog") == 0 )
   {
      iKeyId = atoi(argv[argc-1]);
   }

   log_line_forced_to_file("Test: Using initial message queue key id: %d", iKeyId);
   
   /*
   hw_stop_process("ruby_logger");

   if ( ! hw_process_exists("ruby_logger") )
   {
      if ( iKeyId == LOGGER_MESSAGE_QUEUE_ID )
         hw_execute_ruby_process(NULL, "ruby_logger", NULL, NULL);
      else
      {
         char szParams[128];
         sprintf(szParams, "-id %d", iKeyId);
         hw_execute_ruby_process(NULL, "ruby_logger", szParams, NULL);
      }
      hardware_sleep_ms(300);
   }
   */
   key_t logServiceKey = 0;
   int logServiceMessageQueue = -1;

   logServiceKey = generate_msgqueue_key(iKeyId);
   log_line_forced_to_file("Test: Generated logger msgqueue key: 0x%X", logServiceKey);
   
   logServiceMessageQueue = msgget(logServiceKey, S_IWUSR | S_IWGRP | S_IWOTH);
   if ( logServiceMessageQueue == -1 )
      log_line_forced_to_file("Test: Failed to get logger message queue id, error: %d, [%s]", errno,  strerror(errno));
   else
      log_line_forced_to_file("Test: Got logger message queue id: %d", logServiceMessageQueue);

   type_log_message_buffer msg;
   msg.type = 1;
   msg.text[0] = 0;

   strcpy(msg.text, "Test: Manual log entry sent directly to logger service, waiting for it.");

   if ( msgsnd(logServiceMessageQueue, &msg, strlen(msg.text)+1, 0) < 0 )
      log_line_forced_to_file("Test: Failed to send manual log entry (waiting for it).");
   else
      log_line_forced_to_file("Test: Sent successfully manual log entry (waiting for it)");


   strcpy(msg.text, "Test: Manual log entry sent directly to logger service, no wait.");

   if ( msgsnd(logServiceMessageQueue, &msg, strlen(msg.text)+1, IPC_NOWAIT) < 0 )
      log_line_forced_to_file("Test: Failed to send manual log entry (no wait).");
   else
      log_line_forced_to_file("Test: Sent successfully manual log entry (no wait)");


   log_line("Test log line automated");

   //if ( bStartedLog )
   //   hw_stop_process("ruby_logger");
}


void _check_power_levels_of_current_cards(radio_hw_info_t* pRadioInfoArrayPrev, int iCountPrev)
{
   bool bNewCards = false;

   for( int i=0; i<hardware_get_radio_interfaces_count(); i++ )
   {
      bool bFound = false;
      radio_hw_info_t* pRadioHWInfo = hardware_get_radio_info(i);
      if ( NULL == pRadioHWInfo )
         continue;
      if ( ! pRadioHWInfo->isSupported )
         continue;
      if ( ! hardware_radio_is_wifi_radio(pRadioHWInfo) )
         continue;

      for( int k=0; k<iCountPrev; k++ )
      {
         if ( 0 != pRadioInfoArrayPrev[k].szMAC[0] )
         if ( 0 != pRadioHWInfo->szMAC[0] )
         if ( 0 == strcmp(pRadioHWInfo->szMAC, pRadioInfoArrayPrev[k].szMAC) )
         {
            bFound = true;
            break;
         }
      }

      if ( ! bFound )
      {
         log_line("Radio hardware interface %d: %s (%s, MAC: %s) is new in the system.",
            i+1, pRadioHWInfo->szName, pRadioHWInfo->szDriver, pRadioHWInfo->szMAC);
         bNewCards = true;
      }
   }

   if ( ! bNewCards )
   {
      log_line("No new supported high capacity radio hardware cards found in the system.");
      return;
   }

   log_line("Checking power levels as there are new hardware radio interfaces on the system...");

   {
      bool bUpdated = false;
      int iMaximumRawCanBeSet = 10000;
      for( int i=0; i<hardware_get_radio_interfaces_count(); i++ )
      {
         radio_hw_info_t* pRadioHWInfo = hardware_get_radio_info(i);
         if ( NULL == pRadioHWInfo )
            continue;
         t_ControllerRadioInterfaceInfo* pCRII = controllerGetRadioCardInfo(pRadioHWInfo->szMAC);
         if ( NULL == pCRII )
            continue;

         log_line("Checking radio interface %d: type: %s (%s)",
            i+1, str_get_radio_card_model_string(pCRII->cardModel), (pCRII->cardModel > 0)?"autodetected":"user set");
         int iMaxRawPower = tx_powers_get_max_usable_power_raw_for_card(hardware_getBoardType(), pCRII->cardModel);
         log_line("Max raw power for radio card %d: %d, current raw power: %d", i+1, iMaxRawPower, pCRII->iRawPowerLevel);
      
         if ( iMaxRawPower < iMaximumRawCanBeSet )
            iMaximumRawCanBeSet = iMaxRawPower;
      }
      log_line("Maximum raw power that can be set: %d", iMaximumRawCanBeSet);
      
      for( int i=0; i<hardware_get_radio_interfaces_count(); i++ )
      {
         radio_hw_info_t* pRadioHWInfo = hardware_get_radio_info(i);
         if ( NULL == pRadioHWInfo )
            continue;
         t_ControllerRadioInterfaceInfo* pCRII = controllerGetRadioCardInfo(pRadioHWInfo->szMAC);
         if ( NULL == pCRII )
            continue;

         int iMaxRawPower = tx_powers_get_max_usable_power_raw_for_card(hardware_getBoardType(), pCRII->cardModel);
         if ( pCRII->iRawPowerLevel > iMaxRawPower )
         {
            pCRII->iRawPowerLevel = iMaxRawPower;
            bUpdated = true;
         }
      }
      if ( bUpdated )
         save_ControllerSettings();
   }
   log_line("Done checking power levels.");
}

void _log_platform(bool bNewLine)
{
   printf("Built for Radxa Zero 3W");
   if ( bNewLine )
      printf("\n");
}


int _step_process_cmd_line(int argc, char* argv[])
{
   if ( strcmp(argv[argc-1], "-ver") == 0 )
   {
      printf("%d.%d (b-%d) ", SYSTEM_SW_VERSION_MAJOR, SYSTEM_SW_VERSION_MINOR, SYSTEM_SW_BUILD_NUMBER);
      _log_platform(false);
      return 1;
   }

   if ( strcmp(argv[argc-1], "-resetproc") == 0 )
   {
      printf("\nReset processes settings...\n");
      char szFile[MAX_FILE_PATH_SIZE];
      strcpy(szFile, FOLDER_CONFIG);
      strcat(szFile, FILE_CONFIG_CURRENT_VEHICLE_MODEL);
      if ( modelVehicle.loadFromFile(szFile, true) )
      {
         modelVehicle.resetProcessesParams();
         modelVehicle.saveToFile(szFile, false);
         printf("Done\n");
      }
      return  1;
   }

   if ( strcmp(argv[argc-1], "-disableproc") == 0 )
   {
      printf("\nDisable processes settings...\n");
      char szFile[MAX_FILE_PATH_SIZE];
      strcpy(szFile, FOLDER_CONFIG);
      strcat(szFile, FILE_CONFIG_CURRENT_VEHICLE_MODEL);
      if ( modelVehicle.loadFromFile(szFile, true) )
      {
         modelVehicle.disableProcessesParams();
         modelVehicle.saveToFile(szFile, false);
         printf("Done\n");
      }
      return  1;
   }

   if ( (argc > 1) && (strcmp(argv[1], "-dbgproc") == 0) )
   {
      log_init("RubyDbg");
      hw_log_processes(argc, argv);
      return 1;
   }

   if ( strcmp(argv[argc-1], "-noop") == 0 )
   {
      _log_platform(true);
      return 1;
   }

   if ( (strcmp(argv[argc-1], "-initradio") == 0) || 
        (strcmp(argv[0], "-initradio") == 0) ||
        ((argc>1) && (strcmp(argv[1], "-initradio") == 0)) )
   {
      r_initradio(argc, argv);
      return 1;
   }

   if ( strcmp(argv[argc-1], "-test") == 0 )
   {
      start_test();
      return 1;
   }

   if ( strcmp(argv[argc-1], "-testlog") == 0 )
   {
      _test_log(argc, argv);
      return 1;
   }

   if ( argc >= 2 )
   if ( strcmp(argv[argc-2], "-testlog") == 0 )
   {
      _test_log(argc, argv);
      return 1;
   }

   g_bDebug = false;
   if ( strcmp(argv[argc-1], "-debug") == 0 )
      g_bDebug = true;
   if ( argc >= 2 )
   if ( strcmp(argv[argc-2], "-debug") == 0 )
      g_bDebug = true;

   s_bIgnoreDrivers = false;
   if ( strcmp(argv[argc-1], "-ignoredrivers") == 0 )
      s_bIgnoreDrivers = true;
   if ( argc >= 2 )
   if ( strcmp(argv[argc-2], "-ignoredrivers") == 0 )
      s_bIgnoreDrivers = true;

   return 0;
}


int _step_find_console()
{
   char szComm[1024];
   char szFile[256];
   char *tty_name = ttyname(STDIN_FILENO);
   bool foundGoodConsole = false;
  
   sprintf(szComm, "echo 'Ruby console to start in: [%s]' >> /tmp/ruby_boot.log", ((tty_name != NULL)?tty_name:"N/A"));
   hw_execute_bash_command_silent(szComm, NULL);
   printf("\nRuby: Start on console (%s)\n", ((tty_name != NULL)? tty_name:"N/A"));
   fflush(stdout);

   if ( g_bDebug )
      foundGoodConsole = true;
   if ( (NULL != tty_name) && strcmp(tty_name, "/dev/tty1") == 0 )
      foundGoodConsole = true;
   if ( (NULL != tty_name) && strcmp(tty_name, "/dev/pts/0") == 0 )
      foundGoodConsole = true;


   sprintf(szComm, "echo 'Ruby execute for Radxa Zero 3 platform' >> /tmp/ruby_boot.log");
   hw_execute_bash_command_silent(szComm, NULL);


   if ( !s_managedService && ((NULL == tty_name) || (!foundGoodConsole)) )
   {
      sprintf(szComm, "echo 'Ruby execute in wrong console. Abort.' >> /tmp/ruby_boot.log");
      hw_execute_bash_command_silent(szComm, NULL);
      printf("\nRuby: Try to execute in wrong console (%s). Exiting.\n", tty_name != NULL ? tty_name:"N/A");
      fflush(stdout);
      return 0;
   }
   
   sprintf(szComm, "echo 'Ruby check semaphore...' >> /tmp/ruby_boot.log");
   hw_execute_bash_command_silent(szComm, NULL);
   // systemd owns the service lifetime, including retries within the same boot.
   if (!s_managedService)
      s_pSemaphoreStarted = sem_open("/RUBY_STARTED_SEMAPHORE", O_CREAT | O_EXCL, S_IWUSR | S_IRUSR, 0);
   if ( !s_managedService && ((SEM_FAILED == s_pSemaphoreStarted) || (NULL == s_pSemaphoreStarted)) && (!g_bDebug) )
   {
      printf("\nRuby (v %d.%d b-%d) is starting...\n", SYSTEM_SW_VERSION_MAJOR, SYSTEM_SW_VERSION_MINOR, SYSTEM_SW_BUILD_NUMBER);
      fflush(stdout);
      sleep(8);
      return 0;
   }
   sprintf(szComm, "echo 'Ruby processes are starting...' >> /tmp/ruby_boot.log");
   hw_execute_bash_command_silent(szComm, NULL);

   printf("\nRuby: Is starting...\n");
   fflush(stdout);


   if (!s_managedService)
      system("sudo mount -o remount,rw /");
   hardware_sleep_ms(50);

   sprintf(szComm, "mkdir -p %s", FOLDER_CONFIG);
   hw_execute_bash_command(szComm, NULL);
   if (!s_managedService)
   {
      sprintf(szComm, "chmod 777 %sruby* 2>/dev/null", FOLDER_BINARIES);
      hw_execute_bash_command(szComm, NULL);
      sprintf(szComm, "chmod 777 %sonyx* 2>/dev/null", FOLDER_BINARIES);
      hw_execute_bash_command(szComm, NULL);
      hw_execute_bash_command("chown -hR root *", NULL);
      hw_execute_bash_command("chgrp -hR root *", NULL);
   }

   sprintf(szComm, "chmod 777 %s* 2>/dev/null", FOLDER_CONFIG);
   hw_execute_bash_command(szComm, NULL);

   sprintf(szComm, "mkdir -p %s", FOLDER_CALIBRATION_FILES);
   hw_execute_bash_command(szComm, NULL);
   sprintf(szComm, "chmod 777 %s", FOLDER_CALIBRATION_FILES);
   hw_execute_bash_command(szComm, NULL);

   g_bIsFirstBoot = false;
   strcpy(szFile, FOLDER_CONFIG);
   strcat(szFile, FILE_CONFIG_FIRST_BOOT);
   if ( access( szFile, R_OK ) != -1 )
      g_bIsFirstBoot = true;

   if ( _init_timestamp_and_boot_count() )
      g_bIsFirstBoot = true;

   initLogFiles();

   if ( ! hw_process_exists("ruby_logger") )
   {
      hw_execute_ruby_process(NULL, "ruby_logger", NULL, NULL);
      hardware_sleep_ms(300);
   }

   log_init("RubyStart");
   log_line("Found good console, starting Ruby...");
   return 1;
}


int _step_check_file_system()
{
   char szComm[256];
   char szFile[MAX_FILE_PATH_SIZE];
   bool readWriteOk = false;
   int readWriteRetryCount = 0;
   FILE* fd = NULL;
   
   if (!s_managedService)
   {
      hw_execute_bash_command("date -s 'next year'", NULL);
      hw_execute_bash_command("date -s 'next year'", NULL);
   }

   while ( ! readWriteOk )
   {
      printf("Ruby: Trying to access files...\n");
      readWriteRetryCount++;


      if ( readWriteRetryCount > 50 )
      {
         printf("\nError accessing the file system. Abort.\n\n");
         log_line_forced_to_file("Error accessing the file system. Abort.");

         strcpy(szFile, FOLDER_LOGS);
         strcat(szFile, LOG_FILE_START);
         fd = fopen(szFile, "a+");
         if ( NULL != fd )
         {
            fprintf(fd, "Access to files failed after %d tries. Boot count now: %d\n", readWriteRetryCount, s_iBootCount);
            fclose(fd);
            fd = NULL;
         }
         return -1;
      }
      hardware_sleep_ms(100);


      if (!s_managedService)
      {
         system("sudo mount -o remount,rw /");
         system("sudo mount -o remount,rw /config");
      }
      hardware_sleep_ms(100);

      // For temporary fifo-s, same tmp root folder on all platforms
      sprintf(szComm, "mkdir -p %s", FOLDER_RUBY_FIFO_TEMP);
      hw_execute_bash_command_silent(szComm, NULL);
      sprintf(szComm, "chmod 777 %s 2>/dev/null", FOLDER_RUBY_FIFO_TEMP);
      hw_execute_bash_command_silent(szComm, NULL);
      if ( 0 < strlen(FOLDER_RUBY_FIFO_TEMP) )
      {
         sprintf(szComm, "rm -rf %s* 2>/dev/null", FOLDER_RUBY_FIFO_TEMP);
         hw_execute_bash_command_silent(szComm, NULL);
      }

      sprintf(szComm, "mkdir -p %s", FOLDER_RUBY_TEMP);
      hw_execute_bash_command_silent(szComm, NULL);
      sprintf(szComm, "chmod 777 %s 2>/dev/null", FOLDER_RUBY_TEMP);
      hw_execute_bash_command_silent(szComm, NULL);
      if ( 0 < strlen(FOLDER_RUBY_TEMP) )
      {
         sprintf(szComm, "rm -rf %s* 2>/dev/null", FOLDER_RUBY_TEMP);
         hw_execute_bash_command_silent(szComm, NULL);
      }

      sprintf(szComm, "mkdir -p %sruby", FOLDER_RUBY_TEMP);
      hw_execute_bash_command_silent(szComm, NULL);
      sprintf(szComm, "chmod 777 %sruby", FOLDER_RUBY_TEMP);
      hw_execute_bash_command_silent(szComm, NULL);
      sprintf(szComm, "rm -rf %sruby/* 2>/dev/null", FOLDER_RUBY_TEMP);
      hw_execute_bash_command_silent(szComm, NULL);

      sprintf(szComm, "mkdir -p %s", FOLDER_TEMP_VIDEO_MEM);
      hw_execute_bash_command_silent(szComm, NULL);
      sprintf(szComm, "chmod 777 %s", FOLDER_TEMP_VIDEO_MEM);
      hw_execute_bash_command_silent(szComm, NULL);
      sprintf(szComm, "umount %s", FOLDER_TEMP_VIDEO_MEM);
      hw_execute_bash_command_silent(szComm, NULL);

      sprintf(szComm, "mkdir -p %s", FOLDER_LOGS);
      hw_execute_bash_command_silent(szComm, NULL);
      sprintf(szComm, "chmod 777 %s*", FOLDER_LOGS);
      hw_execute_bash_command_silent(szComm, NULL);

      sprintf(szComm, "mkdir -p %s", FOLDER_CONFIG);
      hw_execute_bash_command_silent(szComm, NULL);
      sprintf(szComm, "chmod 777 %s* 2>/dev/null", FOLDER_CONFIG);
      hw_execute_bash_command(szComm, NULL);

      sprintf(szComm, "mkdir -p %s", FOLDER_CONFIG_MODELS);
      hw_execute_bash_command(szComm, NULL);
      sprintf(szComm, "chmod 777 %s* 2>/dev/null", FOLDER_CONFIG_MODELS);
      hw_execute_bash_command(szComm, NULL);

      sprintf(szComm, "mkdir -p %s", FOLDER_MEDIA);
      hw_execute_bash_command(szComm, NULL);
      sprintf(szComm, "chmod 777 %s", FOLDER_MEDIA);
      hw_execute_bash_command(szComm, NULL);
      sprintf(szComm, "chmod 777 %s* 2>/dev/null", FOLDER_MEDIA);
      hw_execute_bash_command(szComm, NULL);

      sprintf(szComm, "mkdir -p %s", FOLDER_UPDATES);
      hw_execute_bash_command(szComm, NULL);
      sprintf(szComm, "chmod 777 %s", FOLDER_UPDATES);
      hw_execute_bash_command_silent(szComm, NULL);
      sprintf(szComm, "chmod 777 %s* 2>/dev/null", FOLDER_UPDATES);
      hw_execute_bash_command_silent(szComm, NULL);

      sprintf(szComm, "mkdir -p %s%s", FOLDER_UPDATES, SUBFOLDER_UPDATES_PI);
      hw_execute_bash_command(szComm, NULL);
      sprintf(szComm, "chmod 777 %s%s", FOLDER_UPDATES, SUBFOLDER_UPDATES_PI);
      hw_execute_bash_command_silent(szComm, NULL);
      sprintf(szComm, "chmod 777 %s%s* 2>/dev/null", FOLDER_UPDATES, SUBFOLDER_UPDATES_PI);
      hw_execute_bash_command_silent(szComm, NULL);

      sprintf(szComm, "mkdir -p %s%s", FOLDER_UPDATES, SUBFOLDER_UPDATES_RADXA);
      hw_execute_bash_command(szComm, NULL);
      sprintf(szComm, "chmod 777 %s%s", FOLDER_UPDATES, SUBFOLDER_UPDATES_RADXA);
      hw_execute_bash_command_silent(szComm, NULL);
      sprintf(szComm, "chmod 777 %s%s* 2>/dev/null", FOLDER_UPDATES, SUBFOLDER_UPDATES_RADXA);
      hw_execute_bash_command_silent(szComm, NULL);

      sprintf(szComm, "mkdir -p %s%s", FOLDER_UPDATES, SUBFOLDER_UPDATES_OIPC);
      hw_execute_bash_command(szComm, NULL);
      sprintf(szComm, "chmod 777 %s%s", FOLDER_UPDATES, SUBFOLDER_UPDATES_OIPC);
      hw_execute_bash_command_silent(szComm, NULL);
      sprintf(szComm, "chmod 777 %s%s* 2>/dev/null", FOLDER_UPDATES, SUBFOLDER_UPDATES_OIPC);
      hw_execute_bash_command_silent(szComm, NULL);


      sprintf(szComm, "mkdir -p %s%s", FOLDER_UPDATES, SUBFOLDER_UPDATES_DRIVERS);
      hw_execute_bash_command(szComm, NULL);
      sprintf(szComm, "chmod 777 %s%s", FOLDER_UPDATES, SUBFOLDER_UPDATES_DRIVERS);
      hw_execute_bash_command_silent(szComm, NULL);
      sprintf(szComm, "chmod 777 %s%s* 2>/dev/null", FOLDER_UPDATES, SUBFOLDER_UPDATES_DRIVERS);
      hw_execute_bash_command_silent(szComm, NULL);

      sprintf(szComm, "chmod 777 %sres/* 2>/dev/null", FOLDER_BINARIES);
      hw_execute_bash_command_silent(szComm, NULL);

      sprintf(szComm, "mkdir -p %s", FOLDER_OSD_PLUGINS);
      hw_execute_bash_command_silent(szComm, NULL);
      sprintf(szComm, "chmod 777 %s", FOLDER_OSD_PLUGINS);
      hw_execute_bash_command_silent(szComm, NULL);

      sprintf(szComm, "mkdir -p %s", FOLDER_CORE_PLUGINS);
      hw_execute_bash_command_silent(szComm, NULL);
      sprintf(szComm, "chmod 777 %s", FOLDER_CORE_PLUGINS);
      hw_execute_bash_command_silent(szComm, NULL);

      strcpy(szFile, FOLDER_LOGS);
      strcat(szFile, LOG_FILE_START);
      fd = fopen(szFile, "a+");
      if ( NULL == fd )
      {
         printf("Can't access logs folder (%s)\n", FOLDER_LOGS);
         log_line_forced_to_file("Can't access logs folder (%s)", FOLDER_LOGS);
         continue;
      }

      fprintf(fd, "Check for write access (step 1), succeeded on try number: %d (boot count: %d)\n", readWriteRetryCount, s_iBootCount);

      strcpy(szFile, FOLDER_CONFIG);
      strcat(szFile, FILE_CONFIG_BOOT_COUNT);
      FILE* fd2 = fopen(szFile, "wb");
      if ( NULL == fd2 )
      {
         fprintf(fd, "Failed to open boot count config file for write [%s].\n", szFile);
         fclose(fd);
         fd = NULL;
         printf("Can't access config folder (%s)\n", FOLDER_CONFIG);
         continue;
      }
      fprintf(fd2, "%d\n", s_iBootCount);
      fclose(fd2);
      fd2 = NULL;

      fprintf(fd, "Check for write access (step 2), succeeded on try number: %d (boot count: %d)\n", readWriteRetryCount, s_iBootCount);
      fclose(fd);
      fd = NULL;

      readWriteOk = true;
   }

   strcpy(szFile, FOLDER_LOGS);
   strcat(szFile, LOG_FILE_START);
   fd = fopen(szFile, "a+");
   if ( NULL != fd )
   {
      if ( readWriteOk )
         fprintf(fd, "Access to files ok. Boot count now: %d\n", s_iBootCount);
      else
         fprintf(fd, "Access to files failed. Boot count now: %d\n", s_iBootCount);
      fclose(fd);
      fd = NULL;
   }

   printf("Ruby: Access to files: Ok.\n");
   fflush(stdout);

   strcpy(szFile, FOLDER_LOGS);
   strcat(szFile, LOG_FILE_START);
   fd = fopen(szFile, "a+");
   if ( NULL != fd )
   {
      fprintf(fd, "Starting run number %d; Starting Ruby...\n\n", s_iBootCount);
      fclose(fd);
      fd = NULL;
   }
   log_line("Files are ok, checking processes and init log files...");
   return 0;
}

void _step_check_binaries_and_resources()
{
   char szComm[MAX_FILE_PATH_SIZE];

   sprintf(szComm, "rm -rf %s%s", FOLDER_RUBY_TEMP, FILE_TEMP_UPDATE_IN_PROGRESS);
   hw_execute_bash_command(szComm, NULL);

   sprintf(szComm, "rm -rf %s%s", FOLDER_RUBY_TEMP, FILE_TEMP_UPDATE_IN_PROGRESS_APPLY);
   hw_execute_bash_command(szComm, NULL);

   _check_files();


   log_line("Ruby: Start on verison %d.%d (b-%d)", SYSTEM_SW_VERSION_MAJOR, SYSTEM_SW_VERSION_MINOR, SYSTEM_SW_BUILD_NUMBER);
   printf("Ruby: Start on verison %d.%d (b-%d)\n", SYSTEM_SW_VERSION_MAJOR, SYSTEM_SW_VERSION_MINOR, SYSTEM_SW_BUILD_NUMBER);
   fflush(stdout);

   int iMajor, iMinor;
   get_Ruby_BaseVersion(&iMajor, &iMinor);

   log_line("Ruby: Base version is %d.%d", iMajor, iMinor);
   printf("Ruby: Base version is %d.%d\n", iMajor, iMinor);

   char szInfo[256];
   char szOutput[1024];
   strcpy(szInfo, "Ruby: Platform: ");
   strcat(szInfo, "Radxa");

   u32 uBoardType = hardware_getBoardType();
   strcat(szInfo, ", Board: ");
   strcat(szInfo, str_get_hardware_board_name(uBoardType));


   hw_execute_bash_command_raw("uname -rn", szOutput);
   removeTrailingNewLines(szOutput);
   strcat(szInfo, ", ");
   strcat(szInfo, szOutput);   

   log_line(szInfo);
   printf(szInfo);
   printf("\n");
   fflush(stdout);

   if ( access("/usr/share/fonts/truetype/noto/noto.ttf", R_OK) == -1 )
   {
      char szFile[MAX_FILE_PATH_SIZE];
      strcpy(szFile, FOLDER_RESOURCES);
      strcat(szFile, "noto.ttf");
      if ( access(szFile, R_OK) != -1 )
      {
         hw_execute_bash_command("mkdir -p /usr/share/fonts/truetype/noto/", NULL);
         snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "cp -rf %s /usr/share/fonts/truetype/noto/", szFile);
         hw_execute_bash_command(szComm, NULL);
         hw_execute_bash_command("chmod 777 /usr/share/fonts/truetype/noto", NULL);
         hw_execute_bash_command("chmod 777 /usr/share/fonts/truetype/noto/noto.ttf", NULL);
      }
   }

}

void _step_load_init_devices()
{
   hw_execute_bash_command("ip link set wlx down 2>&1 1>/dev/null", NULL);

   hw_execute_bash_command("modprobe i2c-dev", NULL);

   char szOutput[4096];
   hw_execute_bash_command_raw("lsusb", szOutput);
   strcat(szOutput, "\n*END*\n");
   log_line("USB Devices:");
   log_line(szOutput);

   hw_execute_bash_command_raw("lsmod", szOutput);
   strcat(szOutput, "\n*END*\n");
   log_line("Loaded Modules:");
   log_line(szOutput);      
      
   hw_execute_bash_command_raw("i2cdetect -l", szOutput);
   strcat(szOutput, "\n*END*\n");
   log_line("I2C buses:");
   log_line(szOutput);      

   log_line("Ruby: Finding external I2C devices add-ons...");
   printf("Ruby: Finding external I2C devices add-ons...\n");
   fflush(stdout);
   hardware_i2c_reset_enumerated_flag();
   hardware_i2c_enumerate_busses(1);
   // Load existing settings first
   hardware_i2c_load_device_settings();
   hardware_i2c_log_devices();
   // Save existing settings and any new devices
   hardware_i2c_save_device_settings();
   int iKnown = hardware_i2c_get_found_count_known_devices();
   int iConfigurable = hardware_i2c_get_found_count_configurable_devices();
   if ( 0 == iKnown && 0 == iConfigurable )
   {
      log_line("Ruby: Done finding external I2C devices add-ons. None known found." );
      printf("Ruby: Done finding external I2C devices add-ons. None known found.\n" );
   }
   else
   {
      log_line("Ruby: Done finding external I2C devices add-ons. Found %d known devices of which %d are configurable.", iKnown, iConfigurable );
      printf("Ruby: Done finding external I2C devices add-ons. Found %d known devices of which %d are configurable.\n", iKnown, iConfigurable );
   }

   fflush(stdout);

   log_line("Ruby: Finding serial ports...");
   printf("Ruby: Finding serial ports...\n");
   fflush(stdout);

   int iCount = hardware_serial_init_ports();
   log_line("Ruby: Initialized %d serial ports.", iCount);
   printf("Ruby: Initialized %d serial ports\n", iCount);
   fflush(stdout);
}

radio_hw_info_t sRadioInfoPrev[MAX_RADIO_INTERFACES];
int iHwRadiosCountPrev = 0;
int iHwRadiosSupportedCountPrev = 0;


void _step_enumerate_radios()
{
   printf("Ruby: Enumerating supported 2.4/5.8Ghz radio interfaces...\n");
   printf("\n");
   log_line("Ruby: Enumerating supported 2.4/5.8Ghz radio interfaces...");
   fflush(stdout);

   hardware_enumerate_radio_interfaces_step(0);

   //int iCountHighCapacityInterfaces = hardware_get_radio_interfaces_count();

   if ( 0 == hardware_get_radio_interfaces_count() )
   {
      printf("Ruby: No 2.4/5.8 Ghz radio interfaces found!\n");
      printf("\n");
      log_line("Ruby: No 2.4/5.8 Ghz radio interfaces found!");
      fflush(stdout);
   }
   else
   {
      printf("Ruby: %d radio interfaces found on 2.4/5.8 Ghz bands\n", hardware_get_radio_interfaces_count());
      printf("\n");
      log_line("Ruby: %d radio interfaces found on 2.4/5.8 Ghz bands", hardware_get_radio_interfaces_count());
      fflush(stdout);

      printf("Ruby: %d of %d radio interfaces are supported on 2.4/5.8 Ghz bands\n", hardware_get_supported_radio_interfaces_count(), hardware_get_radio_interfaces_count());
      printf("\n");
      log_line("Ruby: %d of %d radio interfaces are supported on 2.4/5.8 Ghz bands", hardware_get_supported_radio_interfaces_count(), hardware_get_radio_interfaces_count());
      fflush(stdout);   
   }
   printf("Ruby: Finding SiK radio interfaces...\n");
   log_line("Ruby: Finding SiK radio interfaces...");
   fflush(stdout);

   hardware_enumerate_radio_interfaces_step(1);

   if ( ! hardware_radio_has_sik_radios() )
   {
      printf("Ruby: No SiK radio interfaces found.\n");
      log_line("Ruby: No SiK radio interfaces found.");
      fflush(stdout);
   }
   else
   {
      printf("Ruby: %d SiK radio interfaces found.\n", hardware_radio_has_sik_radios());
      log_line("Ruby: %d SiK radio interfaces found.", hardware_radio_has_sik_radios());
      fflush(stdout);    
   }

   printf("Ruby: Finding serial radio interfaces...\n");
   log_line("Ruby: Finding serial radio interfaces...");
   fflush(stdout);

   int iCountAdded = hardware_radio_serial_parse_and_add_from_serial_ports_config();

   if ( 0 == iCountAdded )
   {
      printf("Ruby: No serial radio interfaces found.\n");
      log_line("Ruby: No serial radio interfaces found.");
   }
   else
   {
      printf("\nRuby: %d serial radio interfaces found.\n\n", iCountAdded);
      log_line("Ruby: %d serial radio interfaces found.", iCountAdded);
   }
   printf("Ruby: Done finding radio interfaces.\n");
   log_line("Ruby: Done finding radio interfaces.");
   fflush(stdout);
}

void _step_load_init_radios()
{
   // First, store previous configuration;
   // Then, load radio modules (it checks for existing known USB product ids to see what modules to load)
   // Then enumerate and find actual network interfaces
   log_line("Loading previous radio configuration...");
   hardware_load_radio_info_into_buffers(&iHwRadiosCountPrev, &iHwRadiosSupportedCountPrev, &sRadioInfoPrev[0]);
   log_line("Loaded previous radio configuration.");
   hardware_radio_remove_stored_config();
   hardware_reset_radio_enumerated_flag();

   if ( ! s_bIgnoreDrivers )
   {
      hardware_find_usb_radio_interfaces_info();
      hardware_radio_load_radio_modules(1);
   } 
   hardware_sleep_ms(500);

   char szComm[256];
   char szBuff[256];
   char szOutput[4096];
   int iCount = 0;
   bool bWiFiDetected = false;
   int iMaxWifiCardsToDetect = 4;
   int iWifiIndexToTry = 0;

   while ( iCount < iMaxWifiCardsToDetect*2 )
   {      
      szOutput[0] = 0;
      hw_execute_bash_command_raw("lsmod | grep 88", szOutput);
      removeNewLines(szOutput);
      log_line("Content of lsmod: [%s]", szOutput);

      szOutput[0] = 0;
      hw_execute_bash_command_raw("ip link | grep wl", szOutput);
      removeNewLines(szOutput);
      log_line("Content of ip link: [%s]", szOutput);

      szOutput[0] = 0;
      hw_execute_bash_command_raw("ls /sys/class/net/", szOutput);
      removeNewLines(szOutput);
      log_line("Content of class net: [%s]", szOutput);

      sprintf(szBuff, "wlan%d", iWifiIndexToTry);
      if ( NULL != strstr(szOutput, szBuff) )
      {
         log_line("wlan%d detected on the first try: [%s]", iWifiIndexToTry, szOutput);
         bWiFiDetected = true;
      }

      if ( bWiFiDetected )
         break;

      for( int i=0; i<iMaxWifiCardsToDetect; i++ )
      {
         sprintf(szComm, "ip link | grep wlan%d", i);
         hw_execute_bash_command_raw(szComm, szOutput);
         removeNewLines(szOutput);
         if ( NULL != strstr(szOutput, "wlan") )
         {
            sprintf(szComm, "ip link set dev wlan%d down", i );
            hw_execute_bash_command(szComm, NULL);
            hardware_sleep_ms(2*DEFAULT_DELAY_WIFI_CHANGE);
            sprintf(szComm, "ip link set dev wlan%d up", i );
            hw_execute_bash_command(szComm, NULL);
            hardware_sleep_ms(2*DEFAULT_DELAY_WIFI_CHANGE);
         }
      }
      
      log_line("Trying detect wifi cards (try %d)...", iCount);
      printf("Ruby: Trying to detect 2.4/5.8 Ghz cards...\n");
      fflush(stdout);
      
      hardware_sleep_ms(500);
      for( int i=0; i<5; i++ )
      {

         hardware_sleep_ms(170);

         hardware_sleep_ms(170);
      }

      iCount++;
      iWifiIndexToTry++;
      if ( iWifiIndexToTry >= iMaxWifiCardsToDetect )
      {
         iWifiIndexToTry = 0;
         if ( ! s_bIgnoreDrivers )
            hardware_radio_load_radio_modules(1);
         hardware_sleep_ms(1000);
      }
   }

   for( int i=0; i<=iWifiIndexToTry; i++ )
   {
      sprintf(szComm, "ip link 2>&1 | grep wlan%d", i);
      hw_execute_bash_command_raw(szComm, szOutput);
      removeNewLines(szOutput);
      log_line("Radio interface wlan%d state: [%s]", i, szOutput);
   }
   if ( ! bWiFiDetected )
      log_softerror_and_alarm("Failed to find any wifi cards.");

   hw_execute_bash_command_raw("ls /sys/class/net/", szOutput);
   removeNewLines(szOutput);
   log_line("Network devices found: [%s]", szOutput);

   for( int i=0; i<iMaxWifiCardsToDetect; i++ )
   {
      sprintf(szBuff, "/sys/class/net/wlan%d/device/uevent", i);
      if ( access(szBuff, R_OK) != -1 )
      {
         sprintf(szComm, "cat /sys/class/net/wlan%d/device/uevent", i);
         hw_execute_bash_command_raw(szComm, szOutput);
         removeNewLines(szOutput);
         log_line("Network wlan%d info: [%s]", i, szOutput);
      }
   }

   if ( hardware_has_eth() != NULL )
   {
      log_line("Device has an ETH port.");
      printf("Ruby: Device has an ETH port.\n");
   }
   else
   {
      log_line("Device does not have an ETH port.");
      printf("Ruby: Device does not have an ETH port.\n");    
   }
   fflush(stdout);

   _step_enumerate_radios();

}

void handle_sigint(int sig) 
{ 
   log_line("Caught signal to stop: %d\n", sig);
   s_bQuit = true;
} 
  
int main(int argc, char *argv[])
{
   s_managedService = argc == 2 && strcmp(argv[1], "--service") == 0;
   if ( strcmp(argv[argc-1], "-vehicle") == 0 || strcmp(argv[argc-1], "-rx_commands") == 0 || strcmp(argv[argc-1], "-rc") == 0 )
   {
      fprintf(stderr, "This build supports the Radxa Zero 3W ground station only.\n");
      return 1;
   }
   if ( _step_process_cmd_line(argc, argv) )
      return 0;

   signal(SIGPIPE, SIG_IGN);
   signal(SIGINT, handle_sigint);
   signal(SIGTERM, handle_sigint);
   signal(SIGQUIT, handle_sigint);

   char szFile[MAX_FILE_PATH_SIZE];
   char szComm[1204];
   char szOutput[4096];
   szOutput[0] = 0;

   _increase_fast_reboot_counter();
   if ( _get_fast_reboot_counter() > 10 )
      _reset_fast_reboot_counter();

   strcpy(szFile, FOLDER_CONFIG);
   strcat(szFile, "debug");
   if ( access(szFile, R_OK) != -1 )
   {

      for( int i=0; i<10; i++ )
         hardware_sleep_ms(800);

   }

   if ( ! _step_find_console() )
   {
      _reset_fast_reboot_counter();
      return s_managedService ? EXIT_FAILURE : 0;
   }

   log_line_forced_to_file("Found good console. Continuing...");
   log_arguments(argc, argv);
   printf("\nRuby: Start (v %d.%d b-%d) r%d\n", SYSTEM_SW_VERSION_MAJOR, SYSTEM_SW_VERSION_MINOR, SYSTEM_SW_BUILD_NUMBER, s_iBootCount);
   fflush(stdout);

   if ( g_bDebug )
   {
      printf("\nRuby: Start in debug mode\n");
      fflush(stdout);
      hw_execute_bash_command("touch /tmp/debuglog", NULL);
   }

   if ( s_bIgnoreDrivers )
   {
      printf("\nRuby: Ignore drivers instalation\n");
      fflush(stdout);    
   }

   if ( _step_check_file_system() < 0 )
   {
      _reset_fast_reboot_counter();
      return s_managedService ? EXIT_FAILURE : 0;
   }

   FILE* fd = NULL;
   hardware_detectBoardAndSystemType();

   printf("-----------------------------------\n");
   printf("Ruby: System detected as controller.\n");
   fflush(stdout);

   _step_check_binaries_and_resources();


   szOutput[0] = 0;

   if ( g_bIsFirstBoot )
      do_first_boot_pre_initialization(s_bIgnoreDrivers);

   _step_load_init_devices();

   if ( ! g_bIsFirstBoot )
      _check_update_drivers_on_update();
   _step_load_init_radios();

   if ( access( FILE_FORCE_RESET, R_OK ) != -1 )
   {
      unlink(FILE_FORCE_RESET);
      hw_execute_bash_command("rm -rf config/*", NULL);
      sprintf(szComm, "touch %s%s", FOLDER_CONFIG, FILE_CONFIG_FIRST_BOOT);
      hw_execute_bash_command(szComm, NULL);
      hardware_reboot();
      hardware_sleep_ms(900);
   }


   // Detect hardware camera

   hw_execute_ruby_process(NULL, "ruby_initdhcp", NULL, NULL);

   detectSystemType();

   if ( g_bDebug )
   {
       load_Preferences();
       Preferences* pP = get_Preferences();
       pP->nLogLevel = 0;
       save_Preferences();
   }

   hardware_release();

   log_line("Starting Ruby system...");
   fflush(stdout);

   // Reenable serial ports that where used for SiK radio and now are just regular serial ports
   
   bool bSerialPortsUpdated = false;
   for( int i=0; i<hardware_serial_get_ports_count(); i++ )
   {
      hw_serial_port_info_t* pSerialPort = hardware_get_serial_port_info(i);
      if ( NULL == pSerialPort )
         continue;
      if ( pSerialPort->iPortUsage == SERIAL_PORT_USAGE_SIK_RADIO )
      if ( ! hardware_serial_is_sik_radio(pSerialPort->szPortDeviceName) )
      {
         log_line("Serial port %d [%s] was a SiK radio. Now it's a regular serial port. Setting it as used for nothing.", i+1, pSerialPort->szPortDeviceName);
         pSerialPort->iPortUsage = SERIAL_PORT_USAGE_NONE;
         bSerialPortsUpdated = true;
      }
   }
   if ( bSerialPortsUpdated )
   {
      hardware_serial_save_configuration();
   }

   log_line("Running on Radxa Zero 3 hardware");

#ifdef FEATURE_LOCAL_AUDIO_RECORDING
   log_line("Feature local audio recording is: On.");
#else
   log_line("Feature local audio recording is: Off.");
#endif

#ifdef FEATURE_RADIO_SYNCHRONIZE_RXTX_THREADS
   log_line("Feature radio RxTx thread synchronization is: On.");
#else
   log_line("Feature radio RxTx thread synchronization is: Off.");
#endif

   if ( g_bIsFirstBoot )
      do_first_boot_initialization();

   strcpy(szFile, FOLDER_CONFIG);
   strcat(szFile, FILE_CONFIG_CURRENT_VEHICLE_MODEL);
   if ( access( szFile, R_OK) == -1 )
   {
      log_line("Default model/current model is missing. Creating a default one...");
      first_boot_create_default_model();
   }

   {
       // Controller
       hw_execute_bash_command("cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", szOutput);
       if ( NULL == strstr(szOutput, "performance") )
          hardware_set_default_radxa_cpu_freq();


       bool allDisabled = true;
       for( int i=0; i<hardware_get_radio_interfaces_count(); i++ )
       {
          radio_hw_info_t* pNIC = hardware_get_radio_info(i);
          u32 flags = controllerGetCardFlags(pNIC->szMAC);
          if ( ! ( flags & RADIO_HW_CAPABILITY_FLAG_DISABLED ) )
             allDisabled = false;
       }
       if ( allDisabled || (hardware_get_radio_interfaces_count() < 2) )
       if ( hardware_get_radio_interfaces_count() > 0 )
       {
          log_line("There is a single radio card or all are disabled. Enable at least the first card.");
          load_ControllerInterfacesSettings();
          radio_hw_info_t* pNIC = hardware_get_radio_info(0);
          u32 uFlags = controllerGetCardFlags(pNIC->szMAC);
          uFlags &= ~(RADIO_HW_CAPABILITY_FLAG_DISABLED);
          uFlags |= RADIO_HW_CAPABILITY_FLAG_CAN_USE_FOR_VIDEO;
          uFlags |= RADIO_HW_CAPABILITY_FLAG_CAN_USE_FOR_DATA;
          controllerSetCardFlags(pNIC->szMAC, uFlags);
          save_ControllerInterfacesSettings();
          load_ControllerInterfacesSettings();
       }
   }

   hw_execute_ruby_process_wait(NULL, "ruby_start", "-ver", szOutput, 1);
   log_line("ruby_start: [%s]", szOutput);


   {
      hw_execute_ruby_process_wait(NULL, "ruby_rt_station", "-ver", szOutput, 1);
      log_line("ruby_rt_station: [%s]", szOutput);
      hw_execute_ruby_process_wait(NULL, "ruby_rx_telemetry", "-ver", szOutput, 1);
      log_line("ruby_rx_telemetry: [%s]", szOutput);
      hw_execute_ruby_process_wait(NULL, "ruby_tx_rc", "-ver", szOutput, 1);
      log_line("ruby_tx_rc: [%s]", szOutput);
      hw_execute_ruby_process_wait(NULL, "ruby_central", "-ver", szOutput, 1);
      log_line("ruby_central: [%s]", szOutput);
   }

   //hw_stop_process("wpa_supplicant");


   strcpy(szFile, FOLDER_CONFIG);
   strcat(szFile, FILE_CONFIG_CURRENT_VERSION);
   fd = fopen(szFile, "w");
   if ( NULL != fd )
   {
      fprintf(fd, "%u\n", (((u32)SYSTEM_SW_VERSION_MAJOR)<<8) | (u32)SYSTEM_SW_VERSION_MINOR | (((u32)SYSTEM_SW_BUILD_NUMBER)<<16));
      fclose(fd);
   }
   else
      log_softerror_and_alarm("Failed to save current version id to file: %s",FILE_CONFIG_CURRENT_VERSION);

   if ( g_bIsFirstBoot )
   {
      printf("\n\n\n");
      printf("Ruby: First install initialization complete.\n");
      printf("\n\n\n");
      log_line("Ruby: First install initialization complete.");
      fflush(stdout);

      _reset_fast_reboot_counter();

      hw_execute_bash_command("cp -rf /home/radxa/ruby/logs/log_start.txt /home/radxa/ruby/logs/log_firstboot_start.txt", NULL);
      hw_execute_bash_command("cp -rf /home/radxa/ruby/logs/log_system.txt /home/radxa/ruby/logs/log_firstboot.txt", NULL);
      hw_execute_bash_command("sync", NULL);

      hardware_sleep_ms(500);
      if ( !g_bDebug && !s_managedService )
      {
         hardware_reboot();
      }
      if (!s_managedService)
         return 0;
   }
 

   printf("Ruby: Checking for HW changes...");
   log_line("Checking for HW changes...");
   fflush(stdout);


   hardware_i2c_check_and_update_device_settings();
   printf(" Done.\n");
   log_line("Checking for HW changes complete.");
   fflush(stdout);


   ruby_init_ipc_channels();

   
   printf("Ruby: Starting processes...\n");
   log_line("Starting processes...");
   fflush(stdout);

   printf("Ruby: Init radio interfaces...\n");
   log_line("Init radio interfaces...");
   fflush(stdout);

   char szParams[64];
   strcpy(szParams, "-initradio");
   hw_execute_ruby_process_wait(NULL, "ruby_start", szParams, NULL, 1);
   
   log_line("Reloading hardware radio configuration after radio init step completed...");
   hardware_load_radio_info();
   _check_power_levels_of_current_cards(&sRadioInfoPrev[0], iHwRadiosCountPrev);

   printf("Ruby: Starting main process...\n");
   log_line("Starting main process...");
   fflush(stdout);
   
   check_licences();
   
   hw_execute_bash_command_raw("ls /sys/class/net/", szOutput);
   log_line("Network devices found: [%s]", szOutput);

   snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "chmod 777 %s*", FOLDER_CONFIG);
   hw_execute_bash_command(szComm, NULL);

   {

      u32 uControllerId = controller_utils_getControllerId();
      log_line("Controller UID: %u", uControllerId);


      if ( hardware_radio_has_sik_radios() )
      {
         printf("Ruby: Configuring SiK radios...\n");
         log_line("Configuring SiK radios...");
         fflush(stdout);

         load_ControllerSettings();

         strcpy(szFile, FOLDER_CONFIG);
         strcat(szFile, FILE_CONFIG_CURRENT_VEHICLE_MODEL);
         if ( ! modelVehicle.loadFromFile(szFile, true) )
         {
            modelVehicle.resetToDefaults(true);
            modelVehicle.is_spectator = false;
            modelVehicle.saveToFile(szFile, false);
         }
         log_line("Setting default SiK radio params for all SiK radio interfaces on controller...");

         int iSiKRadioLinkIndex = -1;
         for( int i=0; i<modelVehicle.radioLinksParams.links_count; i++ )
         {
            if ( modelVehicle.radioLinkIsSiKRadio(i) )
            {
               iSiKRadioLinkIndex = i;
               break;
            }
         }

         u32 uFreq = DEFAULT_FREQUENCY_433;
         u32 uTxPower = DEFAULT_RADIO_SIK_TX_POWER;
         u32 uDataRate = DEFAULT_RADIO_DATARATE_SIK_AIR;
         u32 uECC = 0;
         u32 uLBT = 0;
         u32 uMCSTR = 0;
         if ( iSiKRadioLinkIndex == -1 )
            log_line("No SiK radio link on current vehicle. Configuring SiK radio interfaces on controller with default parameters.");
         else
         {
            log_line("Current model radio link %d is a SiK radio link. Use it to configure controller.", iSiKRadioLinkIndex+1);
            uFreq = modelVehicle.radioLinksParams.link_frequency_khz[iSiKRadioLinkIndex];
            uDataRate = modelVehicle.radioLinksParams.downlink_datarate_data_bps[iSiKRadioLinkIndex],
            uECC = (modelVehicle.radioLinksParams.link_radio_flags_tx[iSiKRadioLinkIndex] & RADIO_FLAGS_SIK_ECC)?1:0;
            uLBT = (modelVehicle.radioLinksParams.link_radio_flags_tx[iSiKRadioLinkIndex] & RADIO_FLAGS_SIK_LBT)?1:0;
            uMCSTR = (modelVehicle.radioLinksParams.link_radio_flags_tx[iSiKRadioLinkIndex] & RADIO_FLAGS_SIK_MCSTR)?1:0;
         }

         for( int i=0; i<hardware_get_radio_interfaces_count(); i++ )
         {
            if ( hardware_radio_index_is_sik_radio(i) )
            {
               radio_hw_info_t* pRadioHWInfo = hardware_get_radio_info(i);
               if ( NULL == pRadioHWInfo )
               {
                  log_softerror_and_alarm("Failed to get radio hardware info for radio interface %d.", i+1);
                  continue;
               }
               uTxPower = modelVehicle.radioInterfacesParams.interface_raw_power[i];
               hardware_radio_sik_set_frequency_txpower_airspeed_lbt_ecc(pRadioHWInfo,
                  uFreq, uTxPower, uDataRate,
                  uECC, uLBT, uMCSTR,
                  NULL);
            }
         }
         log_line("Setting default SiK radio params for all SiK interfaces on controller. Done.");

         printf("Ruby: Configured SiK radios.\n");
         log_line("Configured SiK radios.");
         fflush(stdout);
      }

      printf("Ruby: Starting controller...\n");
      log_line("Starting controller...");
      fflush(stdout);

      hw_execute_ruby_process(NULL, "ruby_alive", NULL, NULL);
      if (s_managedService)
      {
         _reset_fast_reboot_counter();
         // Preserve the unit's main PID. The watchdog supervises the UI and session.
         execl(FOLDER_BINARIES "ruby_controller", "ruby_controller", (char*)NULL);
         perror("exec ruby_controller");
         return EXIT_FAILURE;
      }
   

      hw_execute_ruby_process(NULL, "ruby_controller", NULL, NULL);

   }

   printf("Ruby: Started processes. Checking if all ok...\n");
   fflush(stdout);

   for( int i=0; i<10; i++ )
      hardware_sleep_ms(500);
   
   log_line("Checking processes start...");
   if ( hw_process_exists("ruby_logger") )
      log_line("ruby_logger is started");
   else
      log_error_and_alarm("ruby_logger is not running");

   {
      int iRetryCounter = 10;
      while ( iRetryCounter > 0 )
      {
         hardware_sleep_ms(2000);
         iRetryCounter--;
         bool bAllOk = false;
         if ( hw_process_exists("ruby_central") )
            log_line("ruby_central is started");
         else
         {
            bAllOk = false;
            log_error_and_alarm("ruby_central is not running");
         }
         if ( hw_process_exists("ruby_controller") )
            log_line("ruby_controller is started");
         else
         {
            bAllOk = false;
            log_error_and_alarm("ruby_controller is not running");
         }
         if ( hw_process_exists("ruby_rt_station") )
            log_line("ruby_rt_station is started");
         else
         {
            bAllOk = false;
            log_error_and_alarm("ruby_rt_station is not running");
         }

         if ( bAllOk )
            break;
      }

      //if ( ! g_bDebug )
      //{
      //   log_line("Clear screen");
      //   system("clear");
      //}
   }

   _reset_fast_reboot_counter();

   hw_execute_bash_command_raw("ls /sys/class/net/", szOutput);
   log_line("Network devices found: [%s]", szOutput);


   hw_execute_bash_command("rm -rf /config/last_ruby_boot.txt", NULL);
   hw_execute_bash_command("cp -rf logs/log_system.txt /config/last_ruby_boot.txt", NULL);
   log_line("Copy boot log to /config partition. Done.");


   if ( NULL != s_pSemaphoreStarted )
      sem_close(s_pSemaphoreStarted);
   log_line("");
   log_line("------------------------------");
   log_line("Ruby Start Finished.");
   log_line("------------------------------");
   log_line("");
   return 0;
}
