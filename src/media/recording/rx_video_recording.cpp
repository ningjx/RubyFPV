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

#include "../byte_queue.h"
#include <atomic>
#include <mutex>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/resource.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../../radio/link/fec.h" 

#include "../../core/base.h"
#include "../../core/config.h"
#include "../../settings/ctrl_settings.h"
#include "../../ipc/shared_mem.h"
#include "../../settings/models.h"
#include "../../radio/radio_utils.h"
#include "../../platform/radxa_zero3w/hardware.h"
#include "../../platform/radxa_zero3w/hardware_procs.h"
#include "../../ipc/ruby_ipc.h"
#include "../parser_h264.h"
#include "../camera_utils.h"
#include "../../core/string_utils.h"
#include "../../radio/link/radiolink.h"
#include "../../protocol/radiopackets2.h"

#include "../../radio/station/shared_vars.h"
#include "rx_video_recording.h"
#include "../../radio/station/packets_utils.h"
#include "../../radio/station/timers.h"
#include "../../radio/station/ruby_rt_station.h"
#include "rx_video_recording_data.h"

std::atomic<bool> s_bIsRecording{false};
std::atomic<bool> s_bRequestedStopRecording{false};
bool s_bIsRecordingToRAM = false;
u32 s_uRecordingLastStartStopTime = 0;

pthread_t s_pThreadVideoRecording;
u32 s_TimeStartRecording = MAX_U32;
char s_szFileRecordingOutput[MAX_FILE_PATH_SIZE];
int s_iFileVideoRecordingOutput = -1;
u32 s_uRecordingFileSize = 0;
int s_iRecordingWidth = 0;
int s_iRecordingHeight = 0;
int s_iRecordingFPS = 0;
int s_iRecordingType = 0;

static ByteQueue<2 * 1024 * 1024> recordingQueue;
static std::mutex recordingMutex;

u32 s_uRecordingStreamCurrentParsedToken = 0x11111111;
bool s_bRecordingFoundStartOfFirstNAL = false;
std::atomic<bool> s_bRecordingThreadReadyForData{false};

void _recording_send_status_to_central(u8 uStatus, u8 uErrorLevel, const char* szError)
{
   t_packet_header PH;
   radio_packet_init(&PH, PACKET_COMPONENT_LOCAL_CONTROL, PACKET_TYPE_LOCAL_CONTROL_VIDEO_RECORDING, STREAM_ID_DATA);
   PH.vehicle_id_src = PACKET_COMPONENT_RUBY;
   PH.vehicle_id_dest = 0;

   PH.total_length = sizeof(t_packet_header) + sizeof(u8);
   if ( uStatus == 0xFF )
   {
      if ( (NULL == szError) || (0 == szError[0]) )
         PH.total_length += 5;
      else
         PH.total_length += 2 + strlen(szError);
   }

   u8 buffer[MAX_PACKET_TOTAL_SIZE];
   memset(buffer, 0, MAX_PACKET_TOTAL_SIZE);
   memcpy(buffer, (u8*)&PH, sizeof(t_packet_header));
   buffer[sizeof(t_packet_header)] = uStatus;
   if ( uStatus == 0xFF )
   {
      if ( (NULL == szError) || (0 == szError[0]) )
         strcpy((char*)(&buffer[sizeof(t_packet_header)+2]), "NA");
      else
      {
         buffer[sizeof(t_packet_header)+1] = uErrorLevel;
         strcpy((char*)(&buffer[sizeof(t_packet_header)+2]), szError);
      }    
   }

   radio_packet_compute_crc(buffer, PH.total_length);
   
   if ( NULL != g_pProcessStats )
      g_pProcessStats->lastIPCOutgoingTime = g_TimeNow;

   if ( ! ruby_ipc_channel_send_message(g_fIPCToCentral, buffer, PH.total_length) )
      log_ipc_send_central_error(buffer, PH.total_length);
   else
      log_line("Sent recording status %d to central.", uStatus);
}

bool _recording_write_info_file(u32 uDurrationMs)
{
   char szFile[MAX_FILE_PATH_SIZE];
   strcpy(szFile, FOLDER_RUBY_TEMP);
   strcat(szFile, FILE_TEMP_VIDEO_FILE_INFO);
   log_line("[VideoRecording-Th] Writing video info file %s ...", szFile);
   FILE* fd = fopen(szFile, "w");
   if ( NULL == fd )
   {
      system("sudo mount -o remount,rw /");
      char szTmp[MAX_FILE_PATH_SIZE];
      sprintf(szTmp, "rm -rf %s%s", FOLDER_RUBY_TEMP, FILE_TEMP_VIDEO_FILE_INFO);
      hw_execute_bash_command(szTmp, NULL);
      fd = fopen(szFile, "w");
   }

   if ( NULL == fd )
   {
      log_softerror_and_alarm("[VideoRecording-Th] Failed to create video info file %s", szFile);
      return false;
   }
   
   fprintf(fd, "%s\n", s_szFileRecordingOutput);
   fprintf(fd, "%d %d\n", s_iRecordingFPS, (int)(uDurrationMs/1000));
   fprintf(fd, "%d %d\n", s_iRecordingWidth, s_iRecordingHeight);
   fprintf(fd, "%d\n", s_iRecordingType);

   int iFC = 0;
   int iOSDFont = 0;
   int iMSPCols = 0;
   int iMSPRows = 0;

   type_global_state_vehicle_runtime_info* pRuntimeInfo = getVehicleRuntimeInfo(g_pCurrentModel->uVehicleId);
   if ( NULL != pRuntimeInfo )
   {
      iFC = pRuntimeInfo->mspState.headerTelemetryMSP.uMSPFlags & MSP_FLAGS_FC_TYPE_MASK;
      iOSDFont = pRuntimeInfo->mspState.headerTelemetryMSP.uMSPFlags & MSP_FLAGS_FC_TYPE_MASK;
      if ( (g_pCurrentModel->osd_params.uFlags & OSD_BIT_FLAGS_MASK_MSPOSD_FONT) != 0 )
         iOSDFont = g_pCurrentModel->osd_params.uFlags & OSD_BIT_FLAGS_MASK_MSPOSD_FONT;
      iMSPCols = pRuntimeInfo->mspState.headerTelemetryMSP.uMSPOSDCols;
      iMSPRows = pRuntimeInfo->mspState.headerTelemetryMSP.uMSPOSDRows;
   }

   fprintf(fd, "%d %d %d %d\n", iFC, iOSDFont, iMSPCols, iMSPRows);
   fclose(fd);

   log_line("[VideoRecording-Th] Created video info file %s, for raw stream: (%s) resolution: %d x %d, %d fps, video type: %d, FC: %d, OSD Font: %d, cols/rows: %d/%d",
      szFile, s_szFileRecordingOutput, s_iRecordingWidth, s_iRecordingHeight, s_iRecordingFPS, s_iRecordingType, iFC, iOSDFont, iMSPCols, iMSPRows);
   return true;
}

void _recording_cleanp_temp_recording_data()
{
   char szComm[512];

   s_uRecordingFileSize = 0;

   if ( 0 != s_szFileRecordingOutput[0] )
   {
      snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "rm -rf %s 2>/dev/null 1>/dev/null", s_szFileRecordingOutput);
      hw_execute_bash_command_silent(szComm, NULL);
      s_szFileRecordingOutput[0] = 0;
   }
   if ( s_bIsRecordingToRAM )
   {
      snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "umount %s", FOLDER_TEMP_VIDEO_MEM);
      hw_execute_bash_command_silent(szComm, NULL);
      s_bIsRecordingToRAM = false;
   }

   snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "rm -rf %s%s 2>/dev/null", FOLDER_RUBY_TEMP, FILE_TEMP_VIDEO_FILE_INFO);
   hw_execute_bash_command(szComm, NULL );
   snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "rm -rf %s%s 2>/dev/null", FOLDER_RUBY_TEMP, FILE_TEMP_VIDEO_FILE_OSD);
   hw_execute_bash_command(szComm, NULL );
   snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "rm -rf %s%s 2>/dev/null", FOLDER_RUBY_TEMP, FILE_TEMP_VIDEO_FILE_SRT);
   hw_execute_bash_command(szComm, NULL );
}

void* _thread_video_recording(void *argument)
{
   log_line("[VideoRecording-Th] Thread to record started.");
   hw_log_current_thread_attributes("video recording");

   s_bRecordingThreadReadyForData = false;

   char szComm[512];
   sprintf(szComm, "mkdir -p %s",FOLDER_MEDIA);
   hw_execute_bash_command(szComm, NULL );
   sprintf(szComm, "chmod 777 %s",FOLDER_MEDIA);
   hw_execute_bash_command(szComm, NULL );

   sprintf(szComm, "chmod 777 %s* 2>/dev/null", FOLDER_MEDIA);
   hw_execute_bash_command(szComm, NULL);

   strcpy(s_szFileRecordingOutput, FOLDER_RUBY_TEMP);
   strcat(s_szFileRecordingOutput, FILE_TEMP_VIDEO_FILE);
   s_bIsRecordingToRAM = false;

   Preferences* p = get_Preferences();
   if ( p->iVideoDestination == prefVideoDestination_Mem )
   {
      strcpy(s_szFileRecordingOutput, FOLDER_TEMP_VIDEO_MEM);
      strcat(s_szFileRecordingOutput, FILE_TEMP_VIDEO_MEM_FILE);
      char szBuff[2048];
      char szTemp[1024];
      sprintf(szComm, "umount %s", FOLDER_TEMP_VIDEO_MEM);
      hw_execute_bash_command_silent(szComm, NULL);

      hw_execute_bash_command_raw("free -m  | grep Mem", szBuff);
      long lt, lu, lf;
      sscanf(szBuff, "%s %ld %ld %ld", szTemp, &lt, &lu, &lf);
      lf -= 200;
      if ( lf > 800 )
         lf -= 200;
      if ( lf < 100 )
      {
         log_line("[VideoRecording-Th] Switched to SD card cache, not enough free ram. Free ram: %d Mb", (int)lf);
         _recording_send_status_to_central(0xFF, 1, "Not enough free memory. Switched recording cache to SD card.");
         strcpy(s_szFileRecordingOutput, FOLDER_RUBY_TEMP);
         strcat(s_szFileRecordingOutput, FILE_TEMP_VIDEO_FILE);
         s_bIsRecordingToRAM = false;
      }
      else
      {
         log_line("[VideoRecording-Th] Create RAM disk of size: %d Mb", (int)lf);
         sprintf(szComm, "mount -t tmpfs -o size=%dM tmpfs %s", (int)lf, FOLDER_TEMP_VIDEO_MEM);
         hw_execute_bash_command(szComm, NULL);
         s_bIsRecordingToRAM = true;
      }
   }

   log_line("[VideoRecording-Th] Recording to output file: (%s)", s_szFileRecordingOutput);
   snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "touch %s", s_szFileRecordingOutput);
   hw_execute_bash_command(szComm, NULL);
   snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "chmod 777 %s", s_szFileRecordingOutput);
   hw_execute_bash_command(szComm, NULL);

   int iOpenFlags = O_CREAT | O_WRONLY;
   //if ( RUBY_PIPES_EXTRA_FLAGS & O_NONBLOCK )
   //   iOpenFlags |= O_NONBLOCK;
   s_iFileVideoRecordingOutput = open(s_szFileRecordingOutput, iOpenFlags);
   if ( -1 == s_iFileVideoRecordingOutput )
   {



      _recording_cleanp_temp_recording_data();
      _recording_send_status_to_central(0xFF, 2, "Recording error. Failed to create recording file.");
      _recording_send_status_to_central(0, 0, NULL);
      s_bRequestedStopRecording = false;
      s_bIsRecording = false;

      log_line("[VideoRecording-Th] Exit due to error creating recording file [%s]", s_szFileRecordingOutput);
      return NULL;
   }

   if ( g_pControllerSettings->iRecordSTR )
      rx_video_recording_data_start_srt();
   if ( g_pControllerSettings->iRecordOSD )
      rx_video_recording_data_start_osd();

   //if ( RUBY_PIPES_EXTRA_FLAGS & O_NONBLOCK )
   //if ( 0 != fcntl(s_iFileVideoRecordingOutput, F_SETFL, O_NONBLOCK) )
   //   log_softerror_and_alarm("[VideoRecording] Failed to set nonblock flag on video recording file");

   log_line("[VideoRecording-Th] Video recording file flags: %s", str_get_pipe_flags(fcntl(s_iFileVideoRecordingOutput, F_GETFL)));

   s_TimeStartRecording = 0;
   s_uRecordingFileSize = 0;
   s_uRecordingStreamCurrentParsedToken = 0x11111111;
   s_bRecordingFoundStartOfFirstNAL = false;

   u8 uRecBuffer[64000];
   u32 uTimeLastVideoMemoryFreeCheck = get_current_timestamp_ms();
   u32 uTimeLastInfoFileWrite = 0;
   bool bFirstWrite = true;
   s_bRecordingThreadReadyForData = true;

   log_line("[VideoRecording-Th] Started waiting for data...");

   while (true)
   {
      u32 uTimeNow = get_current_timestamp_ms();
      if ( s_bIsRecordingToRAM )
      {
         if ( uTimeNow > uTimeLastVideoMemoryFreeCheck + 4000 )
         {
            uTimeLastVideoMemoryFreeCheck = uTimeNow;
            char szBuff[2048];
            char szTemp[64];
            sprintf(szComm, "df %s | sed -n 2p", FOLDER_TEMP_VIDEO_MEM);
            hw_execute_bash_command_raw(szComm, szBuff);
            long lu, lf, lt;
            sscanf(szBuff, "%s %ld %ld %ld", szTemp, &lt, &lu, &lf);
            log_line("[VideoRecording-Th] Free mem disk: %d kb", lf );
            if ( lf/1000 < 20 )
            {
               _recording_send_status_to_central(0xFF, 1, "Video recording RAM cache is full. Stopping recording...");
               s_bRecordingThreadReadyForData = false;
               break;
            }
         }
      }

      if ( uTimeNow > uTimeLastInfoFileWrite + 4000 )
      {
         uTimeLastInfoFileWrite = uTimeNow;
         if ( 0 != s_TimeStartRecording )
            _recording_write_info_file(uTimeNow - s_TimeStartRecording);
      }

      rx_video_recording_periodic_data_loop();

      size_t count;
      {
         std::lock_guard<std::mutex> lock(recordingMutex);
         count = std::min(sizeof(uRecBuffer), recordingQueue.contiguous());
         if (!count && (g_bQuit || s_bRequestedStopRecording)) break;
         memcpy(uRecBuffer, recordingQueue.front(), count);
         recordingQueue.consume(count);
      }
      if (!count) {
         hardware_sleep_ms(5);
         continue;
      }
      if (bFirstWrite) {
         bFirstWrite = false;
         s_TimeStartRecording = get_current_timestamp_ms();
      }
      size_t written = 0;
      while (written < count) {
         ssize_t n = write(s_iFileVideoRecordingOutput, uRecBuffer + written, count - written);
         if (n < 0 && errno == EINTR) continue;
         if (n <= 0) break;
         written += n;
      }
      s_uRecordingFileSize += written;
      if (written != count) {
         _recording_send_status_to_central(0xFF, 1, "Recording stopped: storage write failed.");
         break;
      }
   }
   s_bRecordingThreadReadyForData = false;
   log_line("[VideoRecording-Th] Finishing recording...");



   close(s_iFileVideoRecordingOutput);
   s_iFileVideoRecordingOutput = -1;

   rx_video_recording_data_stop_osd();
   rx_video_recording_data_stop_srt();

   if ( (0 == s_TimeStartRecording) || (s_uRecordingFileSize < 10000) )
   {
      log_line("[VideoRecording-Th] Not recorded anything as first NAL was not found (start time: %u) or size too small (recording size: %u bytes)", s_TimeStartRecording, s_uRecordingFileSize);

      _recording_cleanp_temp_recording_data();
      _recording_send_status_to_central(0xFF, 0, "No recording created. Recording too short.");
      _recording_send_status_to_central(0, 0, NULL);

      s_bRequestedStopRecording = false;
      s_bIsRecording = false;
      log_line("[VideoRecording-Th] Exit recording thread.");
      return NULL;
   }

   u32 uDurrationMs = get_current_timestamp_ms() - s_TimeStartRecording;
   log_line("[VideoRecording-Th] Recording duration: %u ms (%u sec), total %u bytes", uDurrationMs, uDurrationMs/1000, s_uRecordingFileSize);

   if ( uDurrationMs < 2000 )
   {
      log_line("[VideoRecording-Th] Discard recording as duration is too short: %u ms", uDurrationMs);

      _recording_cleanp_temp_recording_data();
      _recording_send_status_to_central(0xFF, 0, "No recording created. Recording too short.");
      _recording_send_status_to_central(0, 0, NULL);

      s_bRequestedStopRecording = false;
      s_bIsRecording = false;
      log_line("[VideoRecording-Th] Exit recording thread.");
      return NULL;
   }

   if ( ! _recording_write_info_file(uDurrationMs) )
   {
      _recording_cleanp_temp_recording_data();
      _recording_send_status_to_central(0xFF, 2, "Failed to save recording info.");
      _recording_send_status_to_central(0, 0, NULL);

      s_bRequestedStopRecording = false;
      s_bIsRecording = false;
      log_line("[VideoRecording-Th] Exit recording thread.");
      return NULL;
   }
   
   _recording_send_status_to_central(2, 0, NULL);
   _recording_send_status_to_central(0xFF, 0, "Processing video recording file...");

   sprintf(szComm, "rm -rf %s%s 2>/dev/null", FOLDER_RUBY_TEMP, FILE_TEMP_VIDEO_FILE_PROCESS_ERROR);
   hw_execute_bash_command(szComm, NULL );

   hw_execute_bash_command_nonblock("./ruby_video_proc", NULL);

   hardware_sleep_ms(900);

   char szPIDs[1024];
   while ( true )
   {
      bool procRunning = false;
      hw_process_get_pids("ruby_video_proc", szPIDs);
      removeTrailingNewLines(szPIDs);
      if ( strlen(szPIDs) > 2 )
         procRunning = true;

      if ( procRunning )
      {
         hardware_sleep_ms(100);
         continue;
      }
      break;
   }
      
   log_line("[VideoRecording-Th] Video processing process finished.");

   char szFile[512];
   strcpy(szFile, FOLDER_RUBY_TEMP);
   strcat(szFile, FILE_TEMP_VIDEO_FILE_PROCESS_ERROR);
   if ( access(szFile, R_OK) == -1 )
      _recording_send_status_to_central(0xFF, 0, "Completed processing video recording file");
   else
   {
      _recording_send_status_to_central(0xFF, 2, "Processing video recording file failed");

      char * line = NULL;
      size_t len = 0;
      ssize_t read;
      FILE* fd = fopen(szFile, "r");

      while ( (NULL != fd) && ((read = getline(&line, &len, fd)) != -1))
      {
        if ( read > 0 )
           _recording_send_status_to_central(0xFF, 2, line);
      }
      if ( NULL != fd )
         fclose(fd);
   }

   sprintf(szComm, "rm -rf %s%s 2>/dev/null", FOLDER_RUBY_TEMP, FILE_TEMP_VIDEO_FILE_PROCESS_ERROR);
   hw_execute_bash_command(szComm, NULL );
  
   log_line("[VideoRecording-Th] Exit recording thread.");

   _recording_send_status_to_central(0, 0, NULL);
   _recording_cleanp_temp_recording_data();
   s_bRequestedStopRecording = false;
   s_bIsRecording = false;
   return NULL;
}

void rx_video_recording_init()
{
   log_line("[VideoRecording] Init start...");

   s_szFileRecordingOutput[0] = 0;
   s_bRequestedStopRecording = false;
   s_bIsRecording = false;
   s_uRecordingFileSize = 0;
   s_iFileVideoRecordingOutput = -1;
   s_uRecordingStreamCurrentParsedToken = 0x11111111;
   s_bRecordingFoundStartOfFirstNAL = false;
   s_bRecordingThreadReadyForData = false;

   char szComm[MAX_FILE_PATH_SIZE];
   sprintf(szComm, "chmod 777 %s 2>/dev/null 1>/dev/null", FOLDER_MEDIA);
   hw_execute_bash_command(szComm, NULL);
   sprintf(szComm, "chmod 777 %s* 2>/dev/null 1>/dev/null", FOLDER_MEDIA);
   hw_execute_bash_command(szComm, NULL);

   load_Preferences();

   log_line("[VideoRecording] Init start complete.");
}

void rx_video_recording_uninit()
{
   log_line("[VideoRecording] Uninit start...");
   if ( rx_video_recording_stop() )
   {
      u32 uTimeStart = g_TimeNow;
      while ( s_bIsRecording || s_bRequestedStopRecording )
      {
          g_TimeNow = get_current_timestamp_ms();
          if ( g_TimeNow > uTimeStart + 2000 )
             break;
          hardware_sleep_ms(100);
      }
   }
   log_line("[VideoRecording] Uninit complete.");
}

void rx_video_recording_start()
{
   s_uRecordingLastStartStopTime = g_TimeNow;
   if ( s_bIsRecording )
   {
      log_line("[VideoRecording] Received request to start recording but recording is already started. Ignore it.");
      return;
   }
   if ( s_bRequestedStopRecording )
   {
      log_line("[VideoRecording] Received request to start recording but recording is processing a request to stop. Ignore it.");
      return;
   }

   log_line("[VideoRecording] Received request to start recording video.");

   { std::lock_guard<std::mutex> lock(recordingMutex); recordingQueue.clear(); }
   s_iRecordingWidth = 1280;
   s_iRecordingHeight = 720;
   s_iRecordingFPS = 0;
   s_iRecordingType = VIDEO_TYPE_H264;
   for( int i=0; i<MAX_VIDEO_PROCESSORS; i++ )
   {
      if( NULL == g_pVideoProcessorRxList[i] )
         break;
      if ( g_pCurrentModel->uVehicleId != g_pVideoProcessorRxList[i]->m_uVehicleId )
         continue;
      s_iRecordingWidth = g_pVideoProcessorRxList[i]->getVideoWidth();
      s_iRecordingHeight = g_pVideoProcessorRxList[i]->getVideoHeight();
      s_iRecordingFPS = g_pVideoProcessorRxList[i]->getVideoFPS();
      s_iRecordingType = g_pVideoProcessorRxList[i]->getVideoType();
      log_line("[VideoRecording] Found info for VID %u: w/h/fps: %dx%d@%d, type: %d", g_pCurrentModel->uVehicleId, s_iRecordingWidth, s_iRecordingHeight, s_iRecordingFPS, s_iRecordingType);
      break;
   }

   if ( 0 == s_iRecordingWidth )
   {
      log_softerror_and_alarm("[VideoRecording] Can't find processor rx video stream info for VID: %u", g_pCurrentModel->uVehicleId);
      _recording_send_status_to_central(0xFF, 2, "Internal recording error. Error code: 1");
      _recording_send_status_to_central(0, 0, NULL);
      s_bRequestedStopRecording = false;
      s_bIsRecording = false;
      return;
   }

   s_bIsRecording = true;
   s_bRequestedStopRecording = false;

   pthread_attr_t attr;
   ControllerSettings* pCS = get_ControllerSettings();
   if ( pCS->iPrioritiesAdjustment && (pCS->iThreadPriorityVideoRecording > 1) && (pCS->iThreadPriorityVideoRecording < 100) )
      hw_init_worker_thread_attrs(&attr, (pCS->iCoresAdjustment?CORE_AFFINITY_CENTRAL_UI:-1), 256000, SCHED_FIFO, pCS->iThreadPriorityVideoRecording, "video_recording");
   else
      hw_init_worker_thread_attrs(&attr, (pCS->iCoresAdjustment?CORE_AFFINITY_CENTRAL_UI:-1), 256000, SCHED_OTHER, 0, "video_recording");

   if ( 0 != pthread_create(&s_pThreadVideoRecording, &attr, &_thread_video_recording, NULL) )
   {
      log_softerror_and_alarm("[VideoRecording] Failed to create recording thread.");
      _recording_send_status_to_central(0xFF, 2, "Internal recording error. Error code: 4");
      _recording_send_status_to_central(0, 0, NULL);
      pthread_attr_destroy(&attr);


      s_bRequestedStopRecording = false;
      s_bIsRecording = false;
      return;
   }
   pthread_attr_destroy(&attr);
   log_line("[VideoRecording] Started thread to do video recording.");
   _recording_send_status_to_central(1, 0, NULL);
   _recording_send_status_to_central(0xFF, 0, "Video recording started");
   return;
}

bool rx_video_recording_stop()
{
   s_uRecordingLastStartStopTime = g_TimeNow;
   if ( s_bRequestedStopRecording )
   {
      log_line("[VideoRecording] Received request to stop recording but recording is already processing a request to stop. Ignore it.");
      return false;
   }

   if ( ! s_bIsRecording )
   {
      log_line("[VideoRecording] Received request to stop recording video but recording is not started. Ignore it.");
      return false;
   }
   s_bRequestedStopRecording = true;
   log_line("[VideoRecording] Signaled recording thread to stop.");
   return true;
}

bool rx_video_is_recording()
{
   return s_bIsRecording;
}

u32  rx_video_recording_get_last_start_stop_time()
{
   return s_uRecordingLastStartStopTime;
}

void rx_video_recording_on_new_data(u8* pData, int iLength)
{
   if (!s_bRecordingThreadReadyForData || !pData || iLength <= 0) return;
   bool accepted;
   {
      std::lock_guard<std::mutex> lock(recordingMutex);
      if (!s_bIsRecording || s_bRequestedStopRecording || !s_bRecordingThreadReadyForData) return;
      while (!s_bRecordingFoundStartOfFirstNAL && iLength > 0) {
         s_uRecordingStreamCurrentParsedToken = (s_uRecordingStreamCurrentParsedToken << 8) | *pData++;
         --iLength;
         if (s_uRecordingStreamCurrentParsedToken == 1) {
            const u8 header[] = {0, 0, 0, 1};
            recordingQueue.append(header, sizeof(header));
            s_bRecordingFoundStartOfFirstNAL = true;
         }
      }
      if (!iLength) return;
      accepted = recordingQueue.append(pData, iLength);
      if (!accepted) s_bRequestedStopRecording = true;
   }
   if (!accepted)
      _recording_send_status_to_central(0xFF, 1, "Recording stopped: storage could not keep up.");

}

void rx_video_recording_periodic_loop()
{
   //static u32 s_TimeLastPeriodicChecksVideoRecording = 0;
   //if ( g_TimeNow < s_TimeLastPeriodicChecksVideoRecording + 200 )
   //   return;
   //s_TimeLastPeriodicChecksVideoRecording = g_TimeNow;
}
