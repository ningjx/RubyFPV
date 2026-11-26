#include "../../src/session/session.h"
#include "../../src/core/base.h"
#include "../../src/platform/radxa_zero3w/hardware_procs.h"
#include "../../src/session/worker_settings.h"
#include <cassert>
#include <map>
#include <string>

// Replace only the OS process and settings adapters. Exercise the same supervisor
// transitions as ruby_controller without starting radio processes on the host.
static std::map<std::string, std::string> running;
static std::string failLaunch;
static int starts, stops;
WorkerPriorities read_worker_priorities() { return {false, 0, 0, 0}; }
void hardware_sleep_ms(u32) {}
int hw_process_exists(const char* program) { return running.count(program); }
void hw_stop_process(const char* program) { stops += running.erase(program); }
void hw_execute_ruby_process(const char*, const char* program, const char* args, char*)
{
   assert(!running.count(program));
   if (program != failLaunch) running.emplace(program, args ? args : "");
   ++starts;
}

int main()
{
   SessionRequest paired;
   paired.mode = SessionMode::Paired;
   paired.vehicle = 42;
   assert(apply_session(paired) && starts == 3);
   assert(apply_session(paired) && starts == 3 && stops == 0);
   running.erase("ruby_rx_telemetry");
   assert(apply_session(paired) && starts == 4 && stops == 0);
   SessionRequest search;
   search.mode = SessionMode::Search;
   assert(!apply_session(search) && current_session().vehicle == 42);
   search.frequency = 5745000;
   assert(apply_session(search) && starts == 7 && stops == 3);
   assert(running.at("ruby_rt_station").find("-search 5745000") == 0);
   assert(running.at("ruby_tx_rc") == "-search");
   assert(apply_session(SessionRequest{}) && running.empty());
   failLaunch = "ruby_tx_rc";
   assert(!apply_session(paired) && running.empty());
   assert(current_session().mode == SessionMode::Stopped);
}
