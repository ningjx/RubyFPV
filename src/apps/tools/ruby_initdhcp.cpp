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
#include "../../platform/radxa_zero3w/hardware.h"
#include "../../platform/radxa_zero3w/hardware_procs.h"
#include "../../settings/ctrl_settings.h"
#include "../../core/string_utils.h"


void configureDHCP()
{
   log_line("DHCP is enabled. Activating it...");
   char szBuff[4096];
   int i=0;

   hw_execute_bash_command("export PATH=/usr/local/bin:${PATH}", NULL);
   hw_execute_bash_command("ip link set eth0 up", NULL);
   log_line("Waiting for ETH up...");
   do
   {
      i++;
      hw_execute_bash_command_silent("cat /sys/class/net/eth0/operstate 2>/dev/null", szBuff);
      removeTrailingNewLines(szBuff);
      if ( strcmp(szBuff, "up") == 0 )
         break;
      hardware_sleep_ms(500);
   } while(i<30);
   log_line("Done waiting for ETH up.");

   szBuff[0] = 0;
   if( access( "/sys/class/net/eth0/carrier", R_OK ) != -1 )
   { 
      hw_execute_bash_command_silent("cat /sys/class/net/eth0/carrier 2>/dev/null | grep 1", szBuff);
      removeTrailingNewLines(szBuff);
   }
   if ( strcmp(szBuff,"1") == 0 )
   {
      log_line("Detected an ethernet connection.");
      char szType[128];

      strcpy(szType, "STATION");

      //ETHCLIENTIP=`ip addr show eth0 | grep -Po 'inet \K[\d.]+'`            
      //echo "Ethernet IP: $ETHCLIENTIP"
      //ping -n -q -c 1 1.1.1.1 

      log_line("DHCP is enabled. Activated it.");
   }
   else
      log_line("No ethernet connection detected!");		
}

void _check_set_fixed_ip()
{
   load_ControllerSettings();
   ControllerSettings* pCS = get_ControllerSettings();
   if ( (pCS == NULL) || (pCS->nUseFixedIP == 0) )
      return;

   char* pszETH = hardware_has_eth();
   if ( NULL == pszETH )
   {
      log_line("Device does not have an ETH port.");
      return;
   }
   log_line("Setting a fixed IP for eth device: %s", pszETH);

   hw_execute_bash_command("export PATH=/usr/local/bin:${PATH}", NULL);
   //hw_execute_bash_command("ip link set dev eth0 up", NULL);
   //execute_bash_command("ip link set dev eth0 down", NULL);

   char szBuff[256];
   szBuff[0] = 0;
   sprintf(szBuff, "ip addr add %d.%d.%d.%d/24 dev %s", (pCS->uFixedIP >> 24 ) & 0xFF, (pCS->uFixedIP >> 16 ) & 0xFF, (pCS->uFixedIP >> 8 ) & 0xFF, pCS->uFixedIP & 0xFF, pszETH );
   if ( 0 != szBuff[0] )
      hw_execute_bash_command(szBuff, NULL);
   log_line("Setting a fixed IP done.");
   sprintf(szBuff, "ip r a default via 192.168.1.1 2>/dev/null");
   hw_execute_bash_command(szBuff, NULL);

   sprintf(szBuff, "ip link set %s up", pszETH);
   hw_execute_bash_command(szBuff, NULL);
   log_line("Added default ETH route done.");
}

int main(int argc, char *argv[])
{
   log_init("RubyDHCPInit");

   bool bNow = false;
   if ( strcmp(argv[argc-1], "-now") == 0 )
      bNow = true;
   
   if ( bNow )
      sleep(1);
   else
   {
      sleep(2);
      sleep(3);
      sleep(3);
   }

   log_line("Finished waiting. Checking DHCP...");

   hardware_detectBoardAndSystemType();

   char szOutput[4096];
   hw_execute_bash_command_raw("ls /sys/class/net/", szOutput);
   log_line("Network devices found: [%s]", szOutput);

   u32 board_type = hardware_getBoardType();
   log_line("Board type: %d -> %s", board_type, str_get_hardware_board_name(board_type));

   bool bHasETH = ((hardware_has_eth() != NULL)?true:false);

   // DHCP
   if ( ( access( "/boot/nodhcp", R_OK ) != -1 ) || (!bHasETH) )
   {
      log_line("DHCP/ETH can't be enabled.");

      if ( ! bHasETH )
      {
         log_line("SBC with no ETH port: no ETH port to enable");
         return 0;
      }

      log_line("DHCP is disabled with /boot/nodhcp option.");
      _check_set_fixed_ip();
      log_line("Done DHCP/ETH Configuration.");
      return 0;
   }

   _check_set_fixed_ip();

   
   hw_execute_bash_command_raw("ls /sys/class/net/", szOutput);
   log_line("Network devices found: [%s]", szOutput);

   hw_execute_bash_command_raw("hostname", szOutput);
   removeTrailingNewLines(szOutput);
   log_line("Hostname: (%s)", szOutput);
   hw_execute_bash_command_raw("hostname -I", szOutput);
   removeTrailingNewLines(szOutput);
   log_line("Hostname IP: (%s)", szOutput);

   hw_execute_bash_command_raw("ip link | grep -A 5 eth", szOutput);
   removeTrailingNewLines(szOutput);
   strcat(szOutput, "***END***");
   log_line("ip link:");
   log_line(szOutput);

   hw_execute_bash_command_raw("ifconfig | grep -A 5 eth", szOutput);
   removeTrailingNewLines(szOutput);
   strcat(szOutput, "***END***");
   log_line("ifconfig:");
   log_line(szOutput);

   hw_execute_bash_command_raw("pidof pump", szOutput);
   removeTrailingNewLines(szOutput);
   log_line("pump proc: (%s)", szOutput);
   hw_execute_bash_command_raw("pgrep pump", szOutput);
   removeTrailingNewLines(szOutput);
   log_line("pump proc: (%s)", szOutput);
   log_line("Done DHCP config process.");
   return (0);
} 