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
#include "menu.h"
#include "menu_objects.h"
#include "menu_controller.h"
#include "menu_text.h"
#include "menu_confirmation.h"
#include "menu_controller_expert.h"
#include "menu_controller_cpu_priorities.h"
#include "menu_item_section.h"
#include "../process_router_messages.h"
#include "../../base/hardware_files.h"
#include <time.h>
#include <sys/resource.h>
#include <semaphore.h>


MenuControllerExpert::MenuControllerExpert(void)
:Menu(MENU_ID_CONTROLLER_EXPERT, L("CPU and Processes Settings"), NULL)
{
   m_Width = 0.34;
   m_xPos = menu_get_XStartPos(m_Width); m_yPos = 0.15;

   addTopInfo();

   m_iIndexPriorities = -1;
   ControllerSettings* pCS = get_ControllerSettings();
   if ( (NULL != pCS) && pCS->iDeveloperMode )
   {
      m_iIndexPriorities = addMenuItem(new MenuItem(L("Processes Priorities"), L("Sets controller processes priorities.")));
      m_pMenuItems[m_iIndexPriorities]->showArrow();
      m_pMenuItems[m_iIndexPriorities]->setTextColor(get_Color_Dev());
   }
   m_IndexVersions = addMenuItem(new MenuItem(L("Modules versions"), L("Get all modules versions.")));
   m_IndexReboot = addMenuItem(new MenuItem(L("Restart"), L("Restarts the controller.")));
}

void MenuControllerExpert::addTopInfo()
{
   char szBuffer[2048];
   char szOutput[1024];
   char szOutput2[1024];

   log_line("Menu Controller Expert: adding info...");

   u32 board_type = hardware_getBoardType();
   const char* szBoard = str_get_hardware_board_name(board_type);
   ruby_signal_alive();

   hw_execute_bash_command_raw("nproc --all", szOutput);
   szOutput[strlen(szOutput)-1] = 0;

   int speed = hardware_get_cpu_speed();
   sprintf(szOutput2, "%d Mhz", speed);

   snprintf(szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]), "%s, %s CPU Cores, %s", szBoard, szOutput, szOutput2);
   addTopLine(szBuffer);

   log_line("Menu Controller Expert: added info.");
}


void MenuControllerExpert::onShow()
{
   Menu::onShow();
}

void MenuControllerExpert::Render()
{
   RenderPrepare();
   float yTop = RenderFrameAndTitle();
   float y = yTop;

   for( int i=0; i<m_ItemsCount; i++ )
      y += RenderItem(i,y);
   RenderEnd(yTop);
}

void MenuControllerExpert::onReturnFromChild(int iChildMenuId, int returnValue)
{
   Menu::onReturnFromChild(iChildMenuId, returnValue);

   if ( (1 == iChildMenuId/1000) && (1 == returnValue) )
   {
      onEventReboot();
      hardware_reboot();
   }
}


void MenuControllerExpert::onSelectItem()
{
   Menu::onSelectItem();
   if ( (-1 == m_SelectedIndex) || (m_pMenuItems[m_SelectedIndex]->isEditing()) )
      return;

   ControllerSettings* pcs = get_ControllerSettings();
   if ( NULL == pcs )
   {
      log_softerror_and_alarm("Failed to get pointer to controller settings structure");
      return;
   }

   if ( m_IndexReboot == m_SelectedIndex )
   {
      if ( g_VehiclesRuntimeInfo[g_iCurrentActiveVehicleRuntimeInfoIndex].bGotFCTelemetry )
      if ( g_VehiclesRuntimeInfo[g_iCurrentActiveVehicleRuntimeInfoIndex].headerFCTelemetry.uFCFlags & FC_TELE_FLAGS_ARMED )
      {
         char szText[256];
         if ( NULL != g_VehiclesRuntimeInfo[g_iCurrentActiveVehicleRuntimeInfoIndex].pModel )
            sprintf(szText, "Your %s is armed. Are you sure you want to reboot the controller?", g_VehiclesRuntimeInfo[g_iCurrentActiveVehicleRuntimeInfoIndex].pModel->getVehicleTypeString());
         else
            strcpy(szText, "Your vehicle is armed. Are you sure you want to reboot the controller?");
         MenuConfirmation* pMC = new MenuConfirmation("Warning! Reboot Confirmation", szText, 1);
         if ( g_pCurrentModel->rc_params.uRCFlags & RC_FLAGS_ENABLED )
         {
            pMC->addTopLine(" ");
            pMC->addTopLine("Warning: You have the RC link enabled, the vehicle flight controller might not go into failsafe mode during reboot.");
         }
         add_menu_to_stack(pMC);
         return;
      }

      onEventReboot();
      hardware_reboot();
      return;
   }

   if ( (m_iIndexPriorities != -1) && (m_iIndexPriorities == m_SelectedIndex) )
   {
      add_menu_to_stack(new MenuControllerCPUPriorities());
      return;
   }

   if ( m_IndexVersions == m_SelectedIndex )
   {
      char szComm[256];
      char szBuff[1024];
      char szOutput[1024];

      Menu* pMenu = new Menu(0,"All Modules Versions",NULL);
      pMenu->m_xPos = 0.32;
      pMenu->m_yPos = 0.17;
      pMenu->m_Width = 0.6;
      
      hw_execute_bash_command_raw_silent("./ruby_start -ver", szOutput);
      removeTrailingNewLines(szOutput);
      snprintf(szBuff, sizeof(szBuff)/sizeof(szBuff[0]), "ruby_start: %s", szOutput);
      pMenu->addTopLine(szBuff);

      hw_execute_bash_command_raw_silent("./ruby_controller -ver", szOutput);
      removeTrailingNewLines(szOutput);
      snprintf(szBuff, sizeof(szBuff)/sizeof(szBuff[0]), "ruby_controller: %s", szOutput);
      pMenu->addTopLine(szBuff);

      hw_execute_bash_command_raw_silent("./ruby_central -ver", szOutput);
      removeTrailingNewLines(szOutput);
      snprintf(szBuff, sizeof(szBuff)/sizeof(szBuff[0]), "ruby_central: %s", szOutput);
      pMenu->addTopLine(szBuff);

      hw_execute_bash_command_raw_silent("./ruby_rt_station -ver", szOutput);
      removeTrailingNewLines(szOutput);
      snprintf(szBuff, sizeof(szBuff)/sizeof(szBuff[0]), "ruby_rt_station: %s", szOutput);
      pMenu->addTopLine(szBuff);

      hw_execute_bash_command_raw_silent("./ruby_rx_telemetry -ver", szOutput);
      removeTrailingNewLines(szOutput);
      snprintf(szBuff, sizeof(szBuff)/sizeof(szBuff[0]), "ruby_rx_telemetry: %s", szOutput);
      pMenu->addTopLine(szBuff);

      hw_execute_bash_command_raw_silent("./ruby_tx_rc -ver", szOutput);
      removeTrailingNewLines(szOutput);
      snprintf(szBuff, sizeof(szBuff)/sizeof(szBuff[0]), "ruby_tx_rc: %s", szOutput);
      pMenu->addTopLine(szBuff);

      hw_execute_bash_command_raw_silent("./ruby_i2c -ver", szOutput);
      removeTrailingNewLines(szOutput);
      snprintf(szBuff, sizeof(szBuff)/sizeof(szBuff[0]), "ruby_i2c: %s", szOutput);
      pMenu->addTopLine(szBuff);

      snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "./%s -ver", VIDEO_PLAYER_PIPE);
      hw_execute_bash_command_raw_silent(szComm, szOutput);
      removeTrailingNewLines(szOutput);
      snprintf(szBuff, sizeof(szBuff)/sizeof(szBuff[0]), "%s: %s", VIDEO_PLAYER_PIPE, szOutput);
      pMenu->addTopLine(szBuff);

      snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "./%s -ver", VIDEO_PLAYER_SM);
      hw_execute_bash_command_raw_silent(szComm, szOutput);
      removeTrailingNewLines(szOutput);
      snprintf(szBuff, sizeof(szBuff)/sizeof(szBuff[0]), "%s: %s", VIDEO_PLAYER_SM, szOutput);
      pMenu->addTopLine(szBuff);

      snprintf(szComm, sizeof(szComm)/sizeof(szComm[0]), "./%s -ver", VIDEO_PLAYER_OFFLINE);
      hw_execute_bash_command_raw_silent(szComm, szOutput);
      removeTrailingNewLines(szOutput);
      snprintf(szBuff, sizeof(szBuff)/sizeof(szBuff[0]), "%s: %s", VIDEO_PLAYER_OFFLINE, szOutput);
      pMenu->addTopLine(szBuff);

      hw_execute_bash_command_raw_silent("./ruby_update_worker -ver", szOutput);
      removeTrailingNewLines(szOutput);
      snprintf(szBuff, sizeof(szBuff)/sizeof(szBuff[0]), "ruby_update_worker: %s", szOutput);
      pMenu->addTopLine(szBuff);

      add_menu_to_stack(pMenu);
      return;
   }

}
