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

#include "../frame_scheduler.h"
#include "../menu/menu_objects.h"
#include "../menu/menu_search.h"
#include "../menu/widgets/menu_item_section.h"
#include "../../core/utils.h"
#include "../../platform/radxa_zero3w/hardware_files.h"
#include "../../radio/link/radiolink.h"
#include "../osd/osd_common.h"
#include "../menu/menu.h"
#include <math.h>
#include "../menu/menu_preferences_buttons.h"
#include "../menu/menu_preferences_ui.h"
#include "../menu/menu_confirmation.h"
#include "../process_router_messages.h"
#include "../render_commands.h"
#include "../keyboard.h"
#include "../link_watch.h"
#include "../../radio/tx_powers.h"
#include <atomic>
#include <memory>
#include <string>

bool Menu::checkCancelUpload()
{
   int iCount = 5;
   bool bBackPressed = false;
   while ( iCount > 0 )
   {
      int iEvent = keyboard_get_next_input_event();
      if ( iEvent == 0 )
         break;
      if ( iEvent == INPUT_EVENT_PRESS_BACK )
         bBackPressed = true; 
      iCount--;
   }
   
   if ( ! bBackPressed )
      return false;

   g_bUpdateInProgress = false;
   render_commands_set_custom_status(NULL);
   log_line("The software update was canceled by user.");
   hardware_sleep_ms(50);
   addMessage(L("Update was canceled."));
   g_nFailedOTAUpdates++;
   return true;
}

static std::atomic<bool> archiveRunning{false};
static std::atomic<bool> s_bThreadGenerateUploadError{false};
static char s_szThreadGenerateUploadErrorString[256];

struct ArchiveInput {
   std::string name;
   u32 board;
};

static void * _thread_generate_upload(void *argument)
{
   char szComm[512];
   std::unique_ptr<ArchiveInput> input(static_cast<ArchiveInput*>(argument));
   const char* szFileNameArchive = input->name.c_str();

   hw_log_current_thread_attributes("generate upload arch");
   log_line("Generating upload archive: %s", szFileNameArchive);

   // Check and create update info file if missing
   char szFile[MAX_FILE_PATH_SIZE];
   strcpy(szFile, FOLDER_CONFIG);
   strcat(szFile, FILE_INFO_LAST_UPDATE);

   if( access(szFile, R_OK) == -1 )
   {
      char szVersionFile[MAX_FILE_PATH_SIZE];
      FILE* fd = try_open_base_version_file(szVersionFile);
      if ( NULL != fd )
      {
         fclose(fd);
         sprintf(szComm, "cp %s %s%s 2>/dev/null", szVersionFile, FOLDER_CONFIG, FILE_INFO_LAST_UPDATE);
         hw_execute_bash_command(szComm, NULL);
      }
      else
      {
         log_line("ThreadGenerateUpload: can't access base version file. Generate new update version file...");
         strcpy(szFile, FOLDER_CONFIG);
         strcat(szFile, FILE_INFO_LAST_UPDATE);
         FILE* fd = fopen(szFile, "w");
         if ( NULL != fd )
         {
            fprintf(fd, "%d.%d\n", SYSTEM_SW_VERSION_MAJOR, SYSTEM_SW_VERSION_MINOR);
            fclose(fd);
         }
      }
   }
   // Done - Check and create update info file if missing

   char szFullPathOutputArchive[MAX_FILE_PATH_SIZE];
   strcpy(szFullPathOutputArchive, FOLDER_UPDATES);
   strcat(szFullPathOutputArchive, szFileNameArchive);

   sprintf(szComm, "rm -rf %s 2>/dev/null", szFullPathOutputArchive);
   hw_execute_bash_command(szComm, NULL);

   char szPathTempUpload[MAX_FILE_PATH_SIZE];
   strcpy(szPathTempUpload, FOLDER_RUBY_TEMP);
   strcat(szPathTempUpload, "tempUploadFiles/");
   sprintf(szComm, "mkdir -p %s", szPathTempUpload);
   hw_execute_bash_command(szComm, NULL);

   // Add binaries to upload archive

   char szFolderLocalUpdateBinaries[MAX_FILE_PATH_SIZE];

   strcpy(szFolderLocalUpdateBinaries, FOLDER_UPDATES);
   if ( hardware_board_is_openipc(input->board) )
      strcat(szFolderLocalUpdateBinaries, SUBFOLDER_UPDATES_OIPC);
   else if ( hardware_board_is_raspberry(input->board) )
      strcat(szFolderLocalUpdateBinaries, SUBFOLDER_UPDATES_PI);
   else if ( hardware_board_is_radxa(input->board) )
      strcat(szFolderLocalUpdateBinaries, SUBFOLDER_UPDATES_RADXA);
   else
      log_error_and_alarm("ThreadGenerateUpload: Invalid vehicle board type: none of the known ones.");

   strcpy(szFile, szFolderLocalUpdateBinaries);
   strcat(szFile, "ruby_start");
   if ( ! hardware_file_check_and_fix_access(szFile) )
   {
      log_softerror_and_alarm("ThreadGenerateUpload: vehicle update binaries are missing from folder: %s", szFolderLocalUpdateBinaries);
      s_bThreadGenerateUploadError = true;
      snprintf(s_szThreadGenerateUploadErrorString, sizeof(s_szThreadGenerateUploadErrorString)/sizeof(s_szThreadGenerateUploadErrorString[0]),
        "Error: Failed to access file (%s)", szFile);
      archiveRunning.store(false);
      return NULL;
   }

   snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "chmod 777 %s* 2>/dev/null", szPathTempUpload);
   hw_execute_bash_command(szComm, NULL);

   snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "cp -rf %s%s %s 2>/dev/null", FOLDER_CONFIG, FILE_INFO_LAST_UPDATE, szPathTempUpload);
   hw_execute_bash_command(szComm, NULL);

   // This is added for updating vehicles older than 11.6, they look for ruby_update_vehicle to execute on update
   snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "cp -rf %sruby_update %sruby_update_vehicle", szFolderLocalUpdateBinaries, szFolderLocalUpdateBinaries);
   hw_execute_bash_command(szComm, NULL);
   snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "chmod 777 %s/ruby_update_vehicle", szFolderLocalUpdateBinaries);
   hw_execute_bash_command(szComm, NULL);

   snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "cp -rf %s* %s 2>/dev/null", szFolderLocalUpdateBinaries, szPathTempUpload);
   hw_execute_bash_command(szComm, NULL);

   // Drivers not needed for now to be uploaded
   /*
   snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "cp -rf %s %s 2>/dev/null", FOLDER_DRIVERS, szPathTempUpload);
   hw_execute_bash_command(szComm, NULL);

   // Remove unnecessary drivers
   if ( hardware_board_is_openipc(input->board & BOARD_TYPE_MASK) )
   {
      snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "rm -rf %s*pi*", szPathTempUpload);
      hw_execute_bash_command(szComm, NULL);
      snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "rm -rf %s*radxa*", szPathTempUpload);
      hw_execute_bash_command(szComm, NULL);
   }
   else if ( hardware_board_is_raspberry(input->board & BOARD_TYPE_MASK) )
   {
      snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "rm -rf %s*oipc*", szPathTempUpload);
      hw_execute_bash_command(szComm, NULL);
      snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "rm -rf %s*radxa*", szPathTempUpload);
      hw_execute_bash_command(szComm, NULL);
   }
   else if ( hardware_board_is_radxa(input->board & BOARD_TYPE_MASK) )
   {
      snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "rm -rf %s*pi*", szPathTempUpload);
      hw_execute_bash_command(szComm, NULL);
      snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "rm -rf %s*oipc*", szPathTempUpload);
      hw_execute_bash_command(szComm, NULL);
   }
   */
   
   if ( hardware_board_is_sigmastar(input->board) )
   {
      snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "cp -rf %smaj* %s 2>/dev/null", szFolderLocalUpdateBinaries, szPathTempUpload);
      hw_execute_bash_command(szComm, NULL);
   }
   snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "cp -rf %scustom* %s 2>/dev/null", szFolderLocalUpdateBinaries, szPathTempUpload);
   hw_execute_bash_command(szComm, NULL);

   if ( hardware_board_is_sigmastar(input->board) )
      sprintf(szComm, "tar -C %s -cf %s . 2>&1", szPathTempUpload, szFullPathOutputArchive);
   else
      sprintf(szComm, "tar -czf %s -C %s . 2>&1", szFullPathOutputArchive, szPathTempUpload);
   hw_execute_bash_command(szComm, NULL);
   
   if ( 0 < strlen(szPathTempUpload) )
   {
      sprintf(szComm, "rm -rf %s*", szPathTempUpload);
      hw_execute_bash_command(szComm, NULL);
   }
   sprintf(szComm, "chmod 777 %s 2>&1", szFullPathOutputArchive);
   hw_execute_bash_command(szComm, NULL);

   char szOutput[4096];
   szOutput[0] = 0;
   sprintf(szComm, "tar -tvf %s 2>&1", szFullPathOutputArchive);
   hw_execute_bash_command(szComm, szOutput);

   log_line("ThreadGenerateUpload: generated tar achive contains: [[[%s]]]", szOutput);
   archiveRunning.store(false);
   return NULL;
}

bool Menu::_generate_upload_archive(char* szArchiveName)
{
   g_TimeNow = get_current_timestamp_ms();
   ruby_signal_alive();

   if (archiveRunning.exchange(true)) return false;
   s_bThreadGenerateUploadError = false;
   s_szThreadGenerateUploadErrorString[0] = 0;
   pthread_t pThread;
   g_pCurrentModel->check_detect_board_type();
   ArchiveInput* input = new ArchiveInput{szArchiveName, g_pCurrentModel->hwCapabilities.uBoardType};
   if (pthread_create(&pThread, NULL, &_thread_generate_upload, input) != 0)
   {
      delete input;
      archiveRunning.store(false);
      render_commands_set_progress_percent(-1, true);
      ruby_resume_watchdog("upload generate failed");
      g_bUpdateInProgress = false;
      addMessage(L("There was an error generating software upload file."));
      return false;
   }
   else
      pthread_detach(pThread);


   // Wait for the thread to finish
   u32 uTimeLastRender = 0;
   while (archiveRunning.load())
   {
      try_read_messages_from_router(5);
      hardware_sleep_ms(5);
      g_TimeNow = get_current_timestamp_ms();
      g_TimeNowMicros = get_current_timestamp_micros();
      if ( checkCancelUpload() )
      {
         render_commands_set_progress_percent(-1, true);
         ruby_resume_watchdog("upload canceled");
         g_bUpdateInProgress = false;
         return false;
      }

      if ( g_TimeNow > (uTimeLastRender+100) )
      {
         uTimeLastRender = g_TimeNow;
         render_commands_set_progress_percent(0, true);
         ui_present_progress(ProgressOverlay::Commands);
      }
   }

   g_TimeNow = get_current_timestamp_ms();
   ruby_signal_alive();

   if ( s_bThreadGenerateUploadError )
   {
      render_commands_set_progress_percent(-1, true);
      ruby_resume_watchdog("upload thread error");
      g_bUpdateInProgress = false;
      return false;
   }
   return true;
}

u8 s_uOTAStatus = 0;
u32 s_uOTACounter = 0;
u32 s_uTimeLastOTACounterChanged = 0;

void Menu::updateOTAStatus(u8 uOTAStatus, u32 uOTACounter)
{
   s_uOTAStatus = uOTAStatus;
   s_uOTACounter = uOTACounter;
   s_uTimeLastOTACounterChanged = g_TimeNow;
}

bool Menu::uploadSoftware()
{
   if (archiveRunning.load()) {
      addMessage(L("The previous update archive is still being prepared."));
      return false;
   }
   log_line("Menu: Start upload procedure for vehicle software version %d.%d (mode: %s)...", get_sw_version_major(g_pCurrentModel), get_sw_version_minor(g_pCurrentModel), g_pCurrentModel->is_spectator?"spectator mode":"control mode");

   ruby_pause_watchdog("uploading software to vehicle");
   render_commands_init();
   g_bUpdateInProgress = true;
   render_commands_set_progress_percent(0, true);
   ui_present_progress(ProgressOverlay::Commands);


   char szArchiveToUpload[MAX_FILE_PATH_SIZE];
   strcpy(szArchiveToUpload, "last_uploaded_archive.tar");

   render_commands_set_custom_status("Generating update archive to upload. Please wait.");
   if ( ! _generate_upload_archive(szArchiveToUpload) )
   {
      render_commands_set_progress_percent(-1, true);
      ruby_resume_watchdog("upload failed");
      g_bUpdateInProgress = false;
      if ( s_bThreadGenerateUploadError )
         addMessage2(0, L("Vehicle update binary files are missing or update procedure changed. Please update (again) your controller."), s_szThreadGenerateUploadErrorString);
      else
         addMessage(L("There was an error generating upload software archive."));
      return false;
   }
   render_commands_set_custom_status(NULL);

   log_line("Generated update archive to upload to vehicle (%s).", szArchiveToUpload);

   s_uOTAStatus = 0;
   s_uOTACounter = 0;
   s_uTimeLastOTACounterChanged = 0;

   render_commands_set_custom_status("Uploading software. Please wait.");
   if ( ! _uploadVehicleUpdate(szArchiveToUpload) )
   {
      render_commands_set_progress_percent(-1, true);
      ruby_resume_watchdog("upload failed");
      g_bUpdateInProgress = false;
      addMessage(L("There was an error updating your vehicle."));
      return false;
   }
   render_commands_set_custom_status(NULL);

  
   log_line("Successfully sent software package to vehicle.");
   bool bProcessingFailed = false;
   char szProcessingError[256];
   szProcessingError[0] = 0;

   render_commands_set_progress_percent(-1, true);
   render_commands_set_custom_status("Processing update on vehicle");

   u32 uTimeLastRender = 0;
   u32 uTimeStartProcessing = g_TimeNow;

   while ( true )
   {
      hardware_sleep_ms(100);
      g_TimeNow = get_current_timestamp_ms();
      g_TimeNowMicros = get_current_timestamp_micros();
      ruby_signal_alive();
      if ( checkCancelUpload() )
      {
         log_line("Update was canceled by user.");
         bProcessingFailed = true;
         break;
      }

      try_read_messages_from_router(50);
      
      bool bTimedOut = false;
      if ( g_TimeNow > uTimeStartProcessing + 1000*300 )
         bTimedOut = true;
      if ( 0 != s_uTimeLastOTACounterChanged )
      if ( g_TimeNow > s_uTimeLastOTACounterChanged + 1000*20 )
         bTimedOut = true;

      if ( bTimedOut )
      {
         log_line("Update has timedout.");
         bProcessingFailed = true;
         break;          
      }

      if ( s_uOTAStatus == OTA_UPDATE_STATUS_START_PROCESSING )
         render_commands_set_custom_status("Start processing the update on the vehicle");
      if ( s_uOTAStatus == OTA_UPDATE_STATUS_UNPACK )
         render_commands_set_custom_status("Unpacking update");
      if ( s_uOTAStatus == OTA_UPDATE_STATUS_UPDATING )
         render_commands_set_custom_status("Updating vehicle");
      if ( s_uOTAStatus == OTA_UPDATE_STATUS_POST_UPDATING )
         render_commands_set_custom_status("Post update");
      if ( s_uOTAStatus == OTA_UPDATE_STATUS_COMPLETED )
         render_commands_set_custom_status("Finished. Cleaning up");
      if ( s_uOTAStatus == OTA_UPDATE_STATUS_REBOOT )
         render_commands_set_custom_status("Finished. Vehicle rebooting");
      if ( s_uOTAStatus == OTA_UPDATE_STATUS_FAILED )
      {
         strcpy(szProcessingError, "Vehicle failed to process the update. Disk error.");
         render_commands_set_custom_status(szProcessingError);
         bProcessingFailed = true;
         break;
      }
      if ( s_uOTAStatus == OTA_UPDATE_STATUS_FAILED_DISK_SPACE )
      {
         strcpy(szProcessingError, "Vehicle failed to process the update. Not enough space on device.");
         render_commands_set_custom_status(szProcessingError);
         break;
      }
      if ( g_TimeNow > (uTimeLastRender+100) )
      {
         uTimeLastRender = g_TimeNow;
         ui_present_progress(ProgressOverlay::Commands);
      }

      if ( s_uOTAStatus == OTA_UPDATE_STATUS_COMPLETED )
         break;
   }

   log_line("Finished software upload part, status: %d", s_uOTAStatus);
   if ( (s_uOTAStatus != OTA_UPDATE_STATUS_COMPLETED) || bProcessingFailed )
   {
      render_commands_set_progress_percent(-1, true);
      render_commands_set_custom_status(NULL);
   }
   if ( bProcessingFailed )
   {
      ruby_resume_watchdog("upload processing failed");
      g_bUpdateInProgress = false;
      send_control_message_to_router(PACKET_TYPE_LOCAL_CONTROL_UPDATE_STOPED,0);
      if ( 0 != szProcessingError[0] )
         addMessage(szProcessingError);
      return false;
   }

   g_nSucceededOTAUpdates++;
   g_bDidAnUpdate = true;
   
   log_line("Upload software: wait for vehicle to reboot...");
   render_commands_set_custom_status("Waiting vehicle to reboot");
   if ( NULL != g_pCurrentModel )
   {
      u32 uTimeLastRender = g_TimeNow = get_current_timestamp_ms();
      while ( s_uTimeLastOTACounterChanged > g_TimeNow - 2000 )
      {
         g_TimeNow = get_current_timestamp_ms();
         try_read_messages_from_router(50);
         ruby_signal_alive();
         if ( g_TimeNow > (uTimeLastRender+100) )
         {
            uTimeLastRender = g_TimeNow;
            ui_present_progress(ProgressOverlay::Commands);
         }
      }
      log_line("No more OTA messages from vehicle. Now wait for vehicle reboot...");
      u32 uTimeStartWait = g_TimeNow = get_current_timestamp_ms();
      while ( link_is_vehicle_online_now(g_pCurrentModel->uVehicleId) )
      {
         g_TimeNow = get_current_timestamp_ms();
         if ( g_TimeNow > uTimeStartWait + 20000 )
         {
            log_softerror_and_alarm("Failed to wait for vehicle to go offline after %u ms. Proceed with finishing upload.", g_TimeNow - uTimeStartWait);
            break;
         }
         try_read_messages_from_router(50);
         ruby_signal_alive();
         if ( g_TimeNow > (uTimeLastRender+100) )
         {
            uTimeLastRender = g_TimeNow;
            ui_present_progress(ProgressOverlay::Commands);
         }
      }
      log_line("Finishing upload: Mark vehicle to sync settings.");
      g_pCurrentModel->b_mustSyncFromVehicle = true;
      g_bSyncModelSettingsOnLinkRecover = true;
   }
   log_line("Upload software: Finished waiting for vehicle to go offline.");

   render_commands_set_progress_percent(-1, true);
   render_commands_set_custom_status(NULL);

   g_bAskedForNegociateRadioLink = false;
   g_bUpdateInProgress = false;
   send_control_message_to_router(PACKET_TYPE_LOCAL_CONTROL_UPDATE_STOPED,0);
   ruby_resume_watchdog("update finished");

   log_line("Finished software upload procedure.");
   return true;
}

MenuItemSelect* Menu::createMenuItemCardModelSelector(const char* szTitle)
{
   MenuItemSelect* pItem = new MenuItemSelect(szTitle, L("Sets the radio interface model."));
   pItem->addSelection("Autodetected");
   pItem->addSelection("Generic");
   for( int i=1; i<50; i++ )
   {
      const char* szDesc = str_get_radio_card_model_string(i);
      if ( (NULL != szDesc) && (0 != szDesc[0]) )
      if ( NULL == strstr(szDesc, "NONAME") )
      if ( NULL == strstr(szDesc, "Generic") )
         pItem->addSelection(szDesc);
   }
   pItem->setIsEditable();
   return pItem;
}

bool Menu::_uploadVehicleUpdate(const char* szArchiveToUpload)
{
   command_packet_sw_package cpswp_cancel;
   cpswp_cancel.type = 1; // 0 - zip, 1 - tar
   cpswp_cancel.total_size = 0;
   cpswp_cancel.file_block_index = MAX_U32;
   cpswp_cancel.is_last_block = false;
   cpswp_cancel.block_length = 0;

   g_bUpdateInProgress = true;
   long lSize = 0;

   char szFile[MAX_FILE_PATH_SIZE];
   strcpy(szFile, FOLDER_UPDATES);
   strcat(szFile, szArchiveToUpload);
   FILE* fd = fopen(szFile, "rb");
   if ( NULL != fd )
   {
      fseek(fd, 0, SEEK_END);
      lSize = ftell(fd);
      fseek(fd, 0, SEEK_SET);
      fclose(fd);
   }

   log_line("Sending to vehicle the update archive (method 6.3): [%s], size: %d bytes", szFile, (int)lSize);

   fd = fopen(szFile, "rb");
   if ( NULL == fd )
   {
      addMessage(L("There was an error generating the software package."));
      g_bUpdateInProgress = false;
      return false;
   }

   u32 blockSize = 1100;
   u32 nPackets = ((u32)lSize) / blockSize;
   if ( lSize > (int)(nPackets * blockSize) )
      nPackets++;

   u8** pPackets = (u8**) malloc(nPackets*sizeof(u8*));
   if ( NULL == pPackets )
   {
      addMessage(L("There was an error generating the upload package."));
      g_bUpdateInProgress = false;
      return false;
   }

   // Read packets in memory

   u32 nTotalPackets = 0;
   for( long l=0; l<lSize; )
   {
      u8* pPacket = (u8*) malloc(1500);
      if ( NULL == pPacket )
      {
         //free(pPackets);
         addMessage("There was an error generating the upload package.");
         g_bUpdateInProgress = false;
         return false;
      }
      command_packet_sw_package* pcpsp = (command_packet_sw_package*)pPacket;

      int nRead = fread(pPacket+sizeof(command_packet_sw_package), 1, blockSize, fd);
      if ( nRead < 0 )
      {
         //free((u8*)pPackets);
         addMessage(L("There was an error generating the upload package."));
         g_bUpdateInProgress = false;
         return false;
      }

      pcpsp->block_length = nRead;
      l += nRead;
      pcpsp->total_size = (u32)lSize;
      pcpsp->file_block_index = nTotalPackets;
      pcpsp->is_last_block = ((l == lSize)?true:false);
      pcpsp->type = 1; // 0 - zip, 1 - tar
      pPackets[nTotalPackets] = pPacket;
      nTotalPackets++;
   }

   log_line("Uploading %d sw segments", nTotalPackets);
   ruby_signal_alive();

   send_control_message_to_router(PACKET_TYPE_LOCAL_CONTROL_UPDATE_STARTED,0);

   g_TimeNow = get_current_timestamp_ms();
   g_TimeNowMicros = get_current_timestamp_micros();
   u32 uTimeLastRender = 0;

   static int s_iAckFrequency = DEFAULT_UPLOAD_PACKET_CONFIRMATION_FREQUENCY;

   int iLastAcknowledgedPacket = -1;
   int iPacketToSend = 0;
   int iCountMaxRetriesForCurrentSegments = 10;
   while ( iPacketToSend < (int)nTotalPackets )
   {
      u8* pPacket = pPackets[iPacketToSend];
      command_packet_sw_package* pcpsp = (command_packet_sw_package*)pPacket;

      bool bWaitAck = true;
      if ( (! pcpsp->is_last_block) && ((iPacketToSend % s_iAckFrequency) != 0 ) )
         bWaitAck = false;
      
      if ( ! bWaitAck )
      {
         log_line("Send sw package block %d of %d", iPacketToSend + 1, nTotalPackets);

         g_TimeNow = get_current_timestamp_ms();
         g_TimeNowMicros = get_current_timestamp_micros();
         ruby_signal_alive();

         for( int k=0; k<2; k++ )
            handle_commands_send_single_oneway_command(0, COMMAND_ID_UPLOAD_SW_TO_VEHICLE63, bWaitAck, pPacket, pcpsp->block_length+sizeof(command_packet_sw_package));
         hardware_sleep_ms(2);
         iPacketToSend++;
         continue;
      }

      u32 commandUID = handle_commands_increment_command_counter();
      u8 resendCounter = 0;
      int waitReplyTime = 100;
      bool gotResponse = false;
      bool responseOk = false;

      // Send packet and wait for acknoledge

      if ( iPacketToSend == (int)nTotalPackets - 1 )
         log_line("Send last sw package with ack, segment %d of %d", iPacketToSend + 1, nTotalPackets);
      else
         log_line("Send sw package with ack, segment %d of %d", iPacketToSend + 1, nTotalPackets);

      do
      {
         g_TimeNow = get_current_timestamp_ms();
         g_TimeNowMicros = get_current_timestamp_micros();
         ruby_signal_alive();

         if ( ! handle_commands_send_command_once_to_vehicle(COMMAND_ID_UPLOAD_SW_TO_VEHICLE63, resendCounter, bWaitAck, pPacket, pcpsp->block_length+sizeof(command_packet_sw_package)) )
         {
            addMessage(L("There was an error uploading the software package."));
            fclose(fd);
            g_nFailedOTAUpdates++;
            for( int i=0; i<5; i++ )
            {
               handle_commands_increment_command_counter();
               handle_commands_send_command_once_to_vehicle(COMMAND_ID_UPLOAD_SW_TO_VEHICLE63, 0, 0, (u8*)&cpswp_cancel, sizeof(command_packet_sw_package));
               hardware_sleep_ms(20);
            }
            send_control_message_to_router(PACKET_TYPE_LOCAL_CONTROL_UPDATE_STOPED,0);
            //for( int i=0; i<nTotalPackets; i++ )
            //   free((u8*)pPackets[i]);
            //free((u8*)pPackets);
            g_bUpdateInProgress = false;
            if ( s_iAckFrequency >= 2 )
               s_iAckFrequency /= 2;
            return false;
         }

         resendCounter++;
         gotResponse = false;
         u32 timeToWaitReply = g_TimeNow + waitReplyTime;
         log_line("Waiting for ACK for SW package segment %d, for %d ms, on retry: %d", iPacketToSend + 1, waitReplyTime, resendCounter-1);

         while ( g_TimeNow < timeToWaitReply && (! gotResponse) )
         {
            if ( checkCancelUpload() )
            {
               fclose(fd);
               for( int i=0; i<5; i++ )
               {
                  handle_commands_increment_command_counter();
                  handle_commands_send_command_once_to_vehicle(COMMAND_ID_UPLOAD_SW_TO_VEHICLE63, 0, 0, (u8*)&cpswp_cancel, sizeof(command_packet_sw_package));
                  hardware_sleep_ms(20);
               }
               send_control_message_to_router(PACKET_TYPE_LOCAL_CONTROL_UPDATE_STOPED,0);
               //for( int i=0; i<nTotalPackets; i++ )
               //   free((u8*)pPackets[i]);
               //free((u8*)pPackets);
               g_bUpdateInProgress = false;

               if ( s_iAckFrequency >= 2 )
                  s_iAckFrequency /= 2;
               return false;
            }

            g_TimeNow = get_current_timestamp_ms();
            g_TimeNowMicros = get_current_timestamp_micros();
            if ( try_read_messages_from_router(waitReplyTime) )
            {
               if ( handle_commands_get_last_command_id_response_received() == commandUID )
               {
                  gotResponse = true;
                  responseOk = handle_commands_last_command_succeeded();
                  log_line("Did got an ACK. Succeded: %s", responseOk?"yes":"no");
                  break;
               }
            }
            log_line("Did not get an ACK from vehicle. Keep waiting...");
         } // finised waiting for ACK

         if ( ! gotResponse )
         {
            log_line("Did not get an ACK from vehicle. Increase wait time.");
            waitReplyTime += 50;
            if ( waitReplyTime > 500 )
               waitReplyTime = 500;
         }
      }
      while ( (resendCounter < 15) && (! gotResponse) );

      // Finished sending and waiting for packet with ACK wait

      g_TimeNow = get_current_timestamp_ms();
      g_TimeNowMicros = get_current_timestamp_micros();
      ruby_signal_alive();

      if ( ! gotResponse )
      {
         log_softerror_and_alarm("Did not get a confirmation from vehicle about the software upload.");
         g_nFailedOTAUpdates++;
         fclose(fd);
         for( int i=0; i<5; i++ )
         {
            handle_commands_increment_command_counter();
            handle_commands_send_command_once_to_vehicle(COMMAND_ID_UPLOAD_SW_TO_VEHICLE63, 0, 0, (u8*)&cpswp_cancel, sizeof(command_packet_sw_package));
            hardware_sleep_ms(20);
         }
         send_control_message_to_router(PACKET_TYPE_LOCAL_CONTROL_UPDATE_STOPED,0);

         //for( int i=0; i<nTotalPackets; i++ )
         //   free((u8*)pPackets[i]);
         //free((u8*)pPackets);
         g_bUpdateInProgress = false;
         return false;
      }

      if ( gotResponse && (!responseOk) )
      {
         // Restart from last confirmed packet
         log_softerror_and_alarm("The software package block (segment index %d) was rejected by the vehicle.", iPacketToSend+1);
         iPacketToSend = iLastAcknowledgedPacket; // will be +1 down below
         iCountMaxRetriesForCurrentSegments--;
         if ( iCountMaxRetriesForCurrentSegments < 0 )
         {
            g_nFailedOTAUpdates++;
            fclose(fd);
            for( int i=0; i<5; i++ )
            {
               handle_commands_increment_command_counter();
               handle_commands_send_command_once_to_vehicle(COMMAND_ID_UPLOAD_SW_TO_VEHICLE63, 0, 0, (u8*)&cpswp_cancel, sizeof(command_packet_sw_package));
               hardware_sleep_ms(20);
            }
            send_control_message_to_router(PACKET_TYPE_LOCAL_CONTROL_UPDATE_STOPED,0);

            g_bUpdateInProgress = false;
            if ( s_iAckFrequency >= 2 )
               s_iAckFrequency /= 2;
            return false;
         }
      }

      if ( gotResponse && responseOk )
      {
         iCountMaxRetriesForCurrentSegments = 10;
         iLastAcknowledgedPacket = iPacketToSend;
         log_line("Got ACK for segment %d", iPacketToSend+1);
      }
      int percent = pcpsp->file_block_index*100/(pcpsp->total_size/blockSize);

      if ( checkCancelUpload() )
      {
         fclose(fd);
         for( int i=0; i<5; i++ )
         {
            handle_commands_increment_command_counter();
            handle_commands_send_command_once_to_vehicle(COMMAND_ID_UPLOAD_SW_TO_VEHICLE63, 0, 0, (u8*)&cpswp_cancel, sizeof(command_packet_sw_package));
            hardware_sleep_ms(20);
         }
         send_control_message_to_router(PACKET_TYPE_LOCAL_CONTROL_UPDATE_STOPED,0);
         //for( int i=0; i<nTotalPackets; i++ )
         //   free((u8*)pPackets[i]);
         //free((u8*)pPackets);
         g_bUpdateInProgress = false;
         if ( s_iAckFrequency >= 2 )
            s_iAckFrequency /= 2;
         return false;
      }

      if ( g_TimeNow > (uTimeLastRender+100) )
      {
         uTimeLastRender = g_TimeNow;
         render_commands_set_progress_percent(percent, true);
         ui_present_progress(ProgressOverlay::Commands);
      }
      iPacketToSend++;
   }

   //for( int i=0; i<nTotalPackets; i++ )
   //   free((u8*)pPackets[i]);
   //free((u8*)pPackets);

   return true;
}
