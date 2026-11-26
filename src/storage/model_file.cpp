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
#include "../platform/radxa_zero3w/hardware_procs.h"

bool Model::reloadIfChanged(bool bLoadStats)
{
   char szFile[MAX_FILE_PATH_SIZE];
   strcpy(szFile, FOLDER_CONFIG);
   strcat(szFile, FILE_CONFIG_CURRENT_VEHICLE_MODEL);
   FILE* fd = fopen(szFile, "r");
   if ( NULL == fd )
      return false;

   int iV, iS = 0;
   if ( 1 == fscanf(fd, "%*s %d", &iV) )
   if ( 1 == fscanf(fd, "%*s %d", &iS) )
   if ( iS != iSaveCount )
   {
      log_line("Model: changed. Reload");
      fclose(fd);
      return loadFromFile(szFile, bLoadStats);
   }
   fclose(fd);
   return true;
}

bool Model::loadFromFile(const char* filename, bool bLoadStats)
{
   char szFileNormal[MAX_FILE_PATH_SIZE];
   char szFileBackup[MAX_FILE_PATH_SIZE];

   strcpy(szFileNormal, filename);
   strcpy(szFileBackup, filename);
   szFileBackup[strlen(szFileBackup)-3] = 'b';
   szFileBackup[strlen(szFileBackup)-2] = 'a';
   szFileBackup[strlen(szFileBackup)-1] = 'k';

   bool bMainFileLoadedOk = false;
   bool bBackupFileLoadedOk = false;

   type_vehicle_stats_info stats;
   memcpy((u8*)&stats, (u8*)&m_Stats, sizeof(type_vehicle_stats_info));

   u32 timeStart = get_current_timestamp_ms();

   int iVersionMain = 0;
   int iVersionBackup = 0;
   FILE* fd = fopen(szFileNormal, "r");
   if ( NULL != fd )
   {
      if ( 1 != fscanf(fd, "%*s %d", &iVersionMain) )
      {
         log_softerror_and_alarm("Load model: Error on version line. Invalid vehicle configuration file: %s", szFileNormal);
         bMainFileLoadedOk = false;
      }
      else
      {
         //log_line("Found model file version: %d.", iVersion);
         if ( 10 == iVersionMain )
            bMainFileLoadedOk = loadVersion10(fd);
         if ( 11 == iVersionMain )
            bMainFileLoadedOk = loadVersion11(fd);
         if ( 12 == iVersionMain )
            bMainFileLoadedOk = loadVersion12(fd);
         if ( bMainFileLoadedOk )
         {
            iLoadedFileVersion = iVersionMain;
         }
         else
            log_softerror_and_alarm("Invalid vehicle configuration file: %s",szFileNormal);
      }
      fclose(fd);
   }
   else
      bMainFileLoadedOk = false;

   if ( bMainFileLoadedOk )
   {
      if ( ! bLoadStats ) 
         memcpy((u8*)&m_Stats, (u8*)&stats, sizeof(type_vehicle_stats_info));

      timeStart = get_current_timestamp_ms() - timeStart;
      char szFreq1[64];
      char szFreq2[64];
      char szFreq3[64];
      strcpy(szFreq1, str_format_frequency(radioLinksParams.link_frequency_khz[0]));
      strcpy(szFreq2, str_format_frequency(radioLinksParams.link_frequency_khz[1]));
      strcpy(szFreq3, str_format_frequency(radioLinksParams.link_frequency_khz[2]));

      log_line("Loaded vehicle (%s) successfully from file: [%s] name: [%s], VID: %u, %s, software: %d.%d (b-%d), has negociated radio: %s, on time: %02d:%02d",
         bLoadStats?"with stats":"without stats",
         filename, vehicle_name, uVehicleId, 
         is_spectator?"spectator mode": "control mode",
         get_sw_version_major(this), get_sw_version_minor(this), get_sw_version_build(this),
         (radioLinksParams.uGlobalRadioLinksFlags & MODEL_RADIOLINKS_FLAGS_HAS_NEGOCIATED_LINKS)?"yes":"no",
         m_Stats.uCurrentOnTime/60, m_Stats.uCurrentOnTime%60);
      constructLongName();
      return true;
   }


   fd = fopen(szFileBackup, "r");
   if ( NULL != fd )
   {
      if ( 1 != fscanf(fd, "%*s %d", &iVersionBackup) )
      {
         log_softerror_and_alarm("Load model: Error on version line. Invalid vehicle configuration file: %s", szFileBackup);
         bBackupFileLoadedOk = false;
      }
      else
      {
         //log_line("Found model file version: %d.", iVersion);
         if ( 10 == iVersionBackup )
            bBackupFileLoadedOk = loadVersion10(fd);
         if ( 11 == iVersionBackup )
            bBackupFileLoadedOk = loadVersion11(fd);
         if ( 12 == iVersionBackup )
            bBackupFileLoadedOk = loadVersion12(fd);
         if ( bBackupFileLoadedOk )
         {
            iLoadedFileVersion = iVersionBackup;
         }
         else
            log_softerror_and_alarm("Invalid vehicle configuration file: %s",szFileBackup);
      }
      fclose(fd);
   }
   else
      bBackupFileLoadedOk = false;

   if ( !bBackupFileLoadedOk )
   {
      if ( ! bLoadStats ) 
         memcpy((u8*)&m_Stats, (u8*)&stats, sizeof(type_vehicle_stats_info));
      log_softerror_and_alarm("Failed to load vehicle configuration from file: %s (missing file, missing backup)",filename);
      resetToDefaults(true);
      return false;
   }

   if ( ! bLoadStats ) 
      memcpy((u8*)&m_Stats, (u8*)&stats, sizeof(type_vehicle_stats_info));

   timeStart = get_current_timestamp_ms() - timeStart;
   log_line("Loaded vehicle successfully (%d ms) from backup file: %s; version %d, save count: %d, vehicle name: [%s], vehicle id: %u, software: %d.%d (b-%d), is in control mode: %s", timeStart, filename, iLoadedFileVersion, iSaveCount, vehicle_name, uVehicleId, get_sw_version_major(this), get_sw_version_minor(this), get_sw_version_build(this), is_spectator?"no (is spectator)":"yes");

   constructLongName();
   
   fd = fopen(szFileNormal, "w");
   if ( NULL != fd )
   {
      saveVersion12(fd, false);
      fclose(fd);
      log_line("Restored main model file from backup model file.");
   }
   else
      log_softerror_and_alarm("Failed to write main model file from backup model file.");

   return true;
}


bool Model::saveToFile(const char* filename, bool isOnController)
{
   iSaveCount++;
   char szBuff[128];
   char szComm[256];

   //u32 timeStart = get_current_timestamp_ms();

   if ( isOnController )
   {
      sprintf(szBuff, FOLDER_VEHICLE_HISTORY, uVehicleId);
      sprintf(szComm, "mkdir -p %s", szBuff);
      hw_execute_bash_command_silent(szComm, NULL);
   }

   for( int i=0; i<(int)strlen(vehicle_name); i++ )
   {
      if ( vehicle_name[i] == '?' || vehicle_name[i] == '%' || vehicle_name[i] == '.' || vehicle_name[i] == '/' || vehicle_name[i] == '\\' )
         vehicle_name[i] = '_';
   }

   FILE* fd = fopen(filename, "w");
   if ( NULL == fd )
   {
      log_softerror_and_alarm("Failed to save model configuration to file: %s",filename);
      return false;
   }
   saveVersion12(fd, isOnController);
   fflush(fd);
   fclose(fd);

   log_line("Saved vehicle successfully to file: [%s] name: [%s], VID: %u, software: %d.%d (b-%d), has negociated radio: %s, is on controller: %s, %s, on time: %02d:%02d",
         filename, vehicle_name, uVehicleId, get_sw_version_major(this), get_sw_version_minor(this), get_sw_version_build(this),
         (radioLinksParams.uGlobalRadioLinksFlags & MODEL_RADIOLINKS_FLAGS_HAS_NEGOCIATED_LINKS)?"yes":"no",
         isOnController?"yes":"no",
         is_spectator?"spectator mode": "control mode",
         m_Stats.uCurrentOnTime/60, m_Stats.uCurrentOnTime%60);
      
   strcpy(szBuff, filename);
   szBuff[strlen(szBuff)-3] = 'b';
   szBuff[strlen(szBuff)-2] = 'a';
   szBuff[strlen(szBuff)-1] = 'k';

   fd = fopen(szBuff, "w");
   if ( NULL == fd )
   {
      log_softerror_and_alarm("Failed to save model configuration to file: %s",szBuff);
      return false;
   }
   saveVersion12(fd, isOnController);
   fflush(fd);
   fclose(fd);

   return true;
}
