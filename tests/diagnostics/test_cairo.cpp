#include "../../src/ipc/shared_mem.h"
#include "../../src/radio/link/radiolink.h"
#include "../../src/protocol/radiopackets2.h"
#include "../../src/core/config.h"
#include "../../src/protocol/commands.h"
#include "../../src/settings/models.h"
#include "../../src/core/string_utils.h"

#include <time.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <poll.h>

bool g_bQuit = false;


void handle_sigint(int sig) 
{ 
   log_line("Caught signal to stop: %d\n", sig);
   g_bQuit = true;
} 
  

int main(int argc, char *argv[])
{
   
   signal(SIGINT, handle_sigint);
   signal(SIGTERM, handle_sigint);
   signal(SIGQUIT, handle_sigint);

   log_init("TestCairo"); 
   log_enable_stdout();
   log_line("Test Cairo UI");
   return (0);
}
