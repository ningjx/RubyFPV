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

#include "launchers_controller.h"
#include <pthread.h>
#include "../core/config.h"
#include "../platform/radxa_zero3w/hardware.h"
#include "../platform/radxa_zero3w/hardware_procs.h"
#include "../settings/ctrl_interfaces.h"
#include "../settings/ctrl_settings.h"
//#include "../base/radio_utils.h"
#include "../protocol/commands.h"
#include "../core/string_utils.h"

#include "shared_vars.h"
#include "popups/popup.h"

static const char* s_szControllerCardErrorFrequency = "You can't set frequency %s, it's used by a different radio link. You would have two radio interfaces transmitting on the same frequency.";
static char s_szControllerCardError[1024];

const char* controller_validate_radio_settings(Model* pModel, u32* pVehicleNICFreq, int* pVehicleNICFlags, int* pVehicleNICRadioFlags, int* pControllerNICFlags, int* pControllerNICRadioFlags)
{
   s_szControllerCardError[0] = 0;

   // Check frequencies

   if ( NULL != pModel && pModel->radioInterfacesParams.interfaces_count > 1 )
   if ( NULL != pVehicleNICFreq )
   {
      for( int i=0; i<pModel->radioInterfacesParams.interfaces_count-1; i++ )
      {
         for( int k=i+1; k<pModel->radioInterfacesParams.interfaces_count; k++ )
         {
            if ( NULL == pVehicleNICFlags )
               continue;
            if ( pVehicleNICFlags[i] & RADIO_HW_CAPABILITY_FLAG_DISABLED )
               continue;
            if ( pVehicleNICFlags[k] & RADIO_HW_CAPABILITY_FLAG_DISABLED )
               continue;

            u32 uFreq1 = pVehicleNICFreq[i];
            u32 uFreq2 = pVehicleNICFreq[k];

            if ( uFreq1 == uFreq2 )
            {
               sprintf(s_szControllerCardError, s_szControllerCardErrorFrequency, str_format_frequency(uFreq1));
               return s_szControllerCardError;
            }
            
            if ( pModel->relay_params.isRelayEnabledOnRadioLinkId >= 0 )
            if ( pModel->relay_params.uRelayedVehicleId != 0 )
            if ( pModel->relay_params.uRelayedVehicleId != MAX_U32 )
            if ( pModel->relay_params.uRelayFrequencyKhz != 0 )
            if ( uFreq1 == pModel->relay_params.uRelayFrequencyKhz )
            {
               sprintf(s_szControllerCardError, "You can't set frequency %s, it's used for relaying.", str_format_frequency(uFreq1));
               return s_szControllerCardError;
            }        
         }
      }
   }
   

   // Check [all cards disabled] on vehicle

   if ( NULL != pModel && NULL != pVehicleNICFlags )
   {
      bool bAllDisabled = true;
      for( int i=0; i<pModel->radioInterfacesParams.interfaces_count; i++ )
         if ( !(pVehicleNICFlags[i] & RADIO_HW_CAPABILITY_FLAG_DISABLED) )
            bAllDisabled = false;
      if ( bAllDisabled )
      {
         strcpy( s_szControllerCardError, "You can't disable all radio links on the vehicle.");
         return s_szControllerCardError;
      }
   }

   // Check missing data capable cards on vehicle

   if ( (NULL != pModel) && (NULL != pVehicleNICFlags) )
   {
      bool bHasData = false;
      for( int i=0; i<pModel->radioInterfacesParams.interfaces_count; i++ )
      {
         if ( pVehicleNICFlags[i] & RADIO_HW_CAPABILITY_FLAG_DISABLED )
            continue;
         if ( pVehicleNICFlags[i] & RADIO_HW_CAPABILITY_FLAG_USED_FOR_RELAY )
            continue;
         //if ( pVehicleNICFlags[i] & RADIO_HW_CAPABILITY_FLAG_CAN_USE_FOR_VIDEO )
         //   bHasVideo = true;
         if ( pVehicleNICFlags[i] & RADIO_HW_CAPABILITY_FLAG_CAN_USE_FOR_DATA )
            bHasData = true;
      }
      if ( ! bHasData )
      {
         strcpy( s_szControllerCardError, "You can't remove all data links from the vehicle.");
         return s_szControllerCardError;
      }
   }

   return NULL;
}


void controller_start_i2c()
{
   char szPrefix[64];
   szPrefix[0] = 0;

   if ( g_pControllerSettings->iPrioritiesAdjustment && (g_pControllerSettings->iThreadPriorityOthers > 100) )
      sprintf(szPrefix, "nice -n %d", g_pControllerSettings->iThreadPriorityOthers - 120);
   hw_execute_ruby_process(szPrefix, "ruby_i2c", NULL, NULL);
}

void controller_stop_i2c()
{
   hw_stop_process("ruby_i2c");
}
