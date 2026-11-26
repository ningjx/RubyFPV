#include "../../src/settings/models.h"
#include <cassert>
#include <unistd.h>

int main()
{
   char directory[] = "/tmp/ruby-model-XXXXXX";
   assert(mkdtemp(directory));
   char file[96], backup[96];
   snprintf(file, sizeof(file), "%s/paired.mdl", directory);
   snprintf(backup, sizeof(backup), "%s/paired.bak", directory);
   Model original;
   original.resetToDefaults(false);
   original.uVehicleId = 7342;
   original.uControllerId = 281;
   original.is_spectator = true;
   strcpy(original.vehicle_name, "station-test");
   assert(original.saveToFile(file, false));
   Model loaded;
   assert(loaded.loadFromFile(file, true));
   assert(loaded.getLoadedFileVersion() == 12 && loaded.uVehicleId == 7342);
   FILE* damaged = fopen(file, "w");
   assert(damaged);
   fputs("truncated\n", damaged);
   fclose(damaged);
   Model recovered;
   assert(recovered.loadFromFile(file, true));
   assert(recovered.uVehicleId == original.uVehicleId && recovered.uControllerId == original.uControllerId);
   // The existing model loader normalizes punctuation in display names.
   assert(recovered.is_spectator && strcmp(recovered.vehicle_name, "station_test") == 0);
   unlink(file);
   unlink(backup);
   rmdir(directory);
}
