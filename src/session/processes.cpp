#include "session.h"
#include "../core/base.h"
#include "../protocol/flags.h"
#include "../platform/radxa_zero3w/hardware_procs.h"
#include "worker_settings.h"
#include <cstdio>
#include <unistd.h>

namespace {
SessionRequest current;
bool cleanupPending = false;
const char* const workers[] = {"ruby_tx_rc", "ruby_rt_station", "ruby_rx_telemetry"};

bool same_session(const SessionRequest& a, const SessionRequest& b)
{
   if (a.mode != b.mode) return false;
   if (a.mode == SessionMode::Paired) return a.vehicle == b.vehicle;
   return a.frequency == b.frequency && a.firmware == b.firmware && a.airRate == b.airRate
      && a.ecc == b.ecc && a.lbt == b.lbt && a.mcstr == b.mcstr;
}

void launch(const char* program, const char* args, int priority, bool adjust)
{
   if (hw_process_exists(program)) return;
   char prefix[48] = {};
   if (adjust && priority > 100) snprintf(prefix, sizeof(prefix), "nice -n %d", priority - 120);
   hw_execute_ruby_process(prefix, program, args, nullptr);
}

bool stop_workers()
{
   cleanupPending = true;
   for (const char* worker : workers) hw_stop_process(worker);
   for (const char* worker : workers)
      if (hw_process_exists(worker)) return false;
   cleanupPending = false;
   current = SessionRequest{};
   return true;
}
}

const SessionRequest& current_session() { return current; }

bool apply_session(const SessionRequest& request)
{
   if (request.mode == SessionMode::Stopped) return stop_workers();
   if (request.mode != SessionMode::Paired && request.mode != SessionMode::Search) return false;
   bool search = request.mode == SessionMode::Search;
   if (search && request.frequency <= 0) return false;
   if ((cleanupPending || (current.mode != SessionMode::Stopped && !same_session(current, request))) && !stop_workers())
      return false;
   char args[128] = {};
   if (search && request.airRate >= 0 && request.firmware == MODEL_FIRMWARE_TYPE_RUBY)
      snprintf(args, sizeof(args), "-search %d -sik %d %d %d %d", request.frequency,
               request.airRate, request.ecc, request.lbt, request.mcstr);
   else if (search)
      snprintf(args, sizeof(args), "-search %d -firmware %d", request.frequency, request.firmware);

   const WorkerPriorities settings = read_worker_priorities();
   launch("ruby_rt_station", args, settings.router, settings.adjust);
   launch("ruby_rx_telemetry", nullptr, settings.telemetry, settings.adjust);
   launch("ruby_tx_rc", search ? "-search" : nullptr, settings.rc, settings.adjust);
   for (int attempt = 0; attempt < 10; ++attempt) {
      if (hw_process_exists("ruby_rt_station") && hw_process_exists("ruby_rx_telemetry")
          && hw_process_exists("ruby_tx_rc")) {
         current = request;
         return true;
      }
      hardware_sleep_ms(20);
   }
   stop_workers();
   return false;
}
