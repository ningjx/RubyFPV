#pragma once

struct WorkerPriorities {
   bool adjust;
   int router, telemetry, rc;
};

WorkerPriorities read_worker_priorities();
