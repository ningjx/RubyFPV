#include "session.h"
#include "../platform/radxa_zero3w/hardware.h"
#include "../platform/radxa_zero3w/hardware_procs.h"
#include "../settings/ctrl_settings.h"
#include "../settings/models.h"
#include <unistd.h>

#include "worker_settings.h"

WorkerPriorities read_worker_priorities()
{
   load_ControllerSettings();
   const auto* settings = get_ControllerSettings();
   return {settings->iPrioritiesAdjustment != 0, settings->iThreadPriorityRouter, settings->iThreadPriorityOthers, settings->iThreadPriorityRC};
}

bool resume_saved_session()
{
   if (access(FOLDER_CONFIG FILE_CONFIG_FIRST_PAIRING_DONE, R_OK) != 0) return false;
   FILE* file = fopen(FOLDER_CONFIG FILE_CONFIG_ACTIVE_CONTROLLER_MODEL, "r");
   if (!file) return false;
   unsigned vehicle = 0;
   bool selected = fscanf(file, "%u", &vehicle) == 1 && vehicle != 0;
   fclose(file);
   if (!selected || hardware_get_radio_interfaces_count() == 0) return false;
   Model model;
   if (!model.loadFromFile(FOLDER_CONFIG FILE_CONFIG_CURRENT_VEHICLE_MODEL, true)
       || model.uVehicleId != vehicle) return false;
   SessionRequest request;
   request.mode = SessionMode::Paired;
   request.vehicle = vehicle;
   return apply_session(request);
}
