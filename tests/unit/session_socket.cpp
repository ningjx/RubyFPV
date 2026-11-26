#include "../../src/session/session.h"
#include <cassert>
#include <cstdio>
#include <poll.h>
#include <thread>
#include <unistd.h>

static bool accept_paired(const SessionRequest& request)
{
   return request.mode == SessionMode::Paired && request.vehicle == 42;
}

int main()
{
   char name[64];
   snprintf(name, sizeof(name), "ruby-session-test-%d", getpid());
   SessionRequest desired;
   desired.vehicle = 42;
   desired.mode = SessionMode::Paired;
   assert(!request_session(desired, name));
   int listener = open_session_listener(name);
   assert(listener >= 0 && open_session_listener(name) == -1);
   std::thread server([listener] {
      pollfd event{listener, POLLIN, 0};
      SessionRequest active;
      active.mode = SessionMode::Paired;
      active.vehicle = 42;
      for (int i = 0; i < 4; ++i) {
         assert(poll(&event, 1, 2000) == 1);
         serve_session_request(listener, accept_paired, active);
      }
   });
   assert(request_session(desired, name));
   SessionRequest observed;
   assert(read_session(observed, name) && observed.vehicle == 42 && observed.mode == SessionMode::Paired);
   assert(request_session(desired, name));
   desired.vehicle = 17;
   assert(!request_session(desired, name));
   server.join();
   close(listener);
   assert(!request_session(desired, name));
}
