#include "../../core/base.h"
#include "../../core/config.h"
#include "hardware_radio_txpower.h"
#include "hardware_radio.h"
#include "hardware_procs.h"

void hardware_radio_set_txpower_raw_rtl8812au(int iCardIndex, int iTxPower)
{
   log_line("Setting radio interface %d RTL8812AU raw tx power to %d using iw dev...", iCardIndex+1, iTxPower);
   if ( (iTxPower < 1) || (iTxPower > MAX_TX_POWER) )
      iTxPower = DEFAULT_RADIO_TX_POWER;

   char szComm[256];
   for( int i=0; i<hardware_get_radio_interfaces_count(); i++ )
   {
      if ( (iCardIndex != -1) && (iCardIndex != i) )
         continue;

      radio_hw_info_t* pRadioHWInfo = hardware_get_radio_info(i);
      if ( NULL == pRadioHWInfo )
         continue;
      if ( (hardware_radio_driver_is_rtl8812au_card(pRadioHWInfo->iRadioDriver)) ||
           (pRadioHWInfo->iRadioType == RADIO_TYPE_RALINK) )
      {
         sprintf(szComm, "iw dev %s set txpower fixed %d", pRadioHWInfo->szName, -100*iTxPower);
         //hw_execute_process_wait(szComm);
         hw_execute_bash_command(szComm, NULL);
      }
   }

   log_line("Radio interface %d RTL8812AU raw tx power changed to: %d", iCardIndex+1, iTxPower);
}

void hardware_radio_set_txpower_raw_rtl8812eu(int iCardIndex, int iTxPower)
{
   log_line("Setting radio interface %d RTL8812EU raw tx power to: %d", iCardIndex+1, iTxPower);
   if ( (iTxPower < 1) || (iTxPower > MAX_TX_POWER) )
      iTxPower = DEFAULT_RADIO_TX_POWER;

   char szComm[256];
   for( int i=0; i<hardware_get_radio_interfaces_count(); i++ )
   {
      if ( (iCardIndex != -1) && (iCardIndex != i) )
         continue;
      radio_hw_info_t* pRadioHWInfo = hardware_get_radio_info(i);
      if ( NULL == pRadioHWInfo )
         continue;
      if ( hardware_radio_driver_is_rtl8812eu_card(pRadioHWInfo->iRadioDriver) )
      {
         sprintf(szComm, "iw dev %s set txpower fixed %d", pRadioHWInfo->szName, iTxPower*40);
         hw_execute_bash_command(szComm, NULL);
      }
   }

   log_line("Radio interface %d RTL8812EU raw tx power changed to: %d", iCardIndex+1, iTxPower);
}

void hardware_radio_set_txpower_raw_rtl8733bu(int iCardIndex, int iTxPower)
{
   log_line("Setting radio interface %d RTL8733BU raw tx power to: %d", iCardIndex+1, iTxPower);
   if ( (iTxPower < 1) || (iTxPower > MAX_TX_POWER) )
      iTxPower = DEFAULT_RADIO_TX_POWER;

   char szComm[256];
   for( int i=0; i<hardware_get_radio_interfaces_count(); i++ )
   {
      if ( (iCardIndex != -1) && (iCardIndex != i) )
         continue;
      radio_hw_info_t* pRadioHWInfo = hardware_get_radio_info(i);
      if ( NULL == pRadioHWInfo )
         continue;
      if ( hardware_radio_driver_is_rtl8733bu_card(pRadioHWInfo->iRadioDriver) )
      {
         sprintf(szComm, "iw dev %s set txpower fixed %d", pRadioHWInfo->szName, iTxPower*40);
         hw_execute_bash_command(szComm, NULL);
      }
   }

   log_line("Radio interface %d RTL8733BU raw tx power changed to: %d", iCardIndex+1, iTxPower);
}

void hardware_radio_set_txpower_raw_atheros(int iCardIndex, int iTxPower)
{
   log_line("Setting radio interface %d Atheros raw tx power to: %d", iCardIndex+1, iTxPower);
   if ( (iTxPower < 1) || (iTxPower > MAX_TX_POWER) )
      iTxPower = DEFAULT_RADIO_TX_POWER;

   log_line("Radio interface %d Atheros raw tx power changed to: %d", iCardIndex+1, iTxPower);
}

