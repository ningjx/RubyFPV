#pragma once
#include "byte_queue.h"
#include <mutex>
#include <stdint.h>

// One instance per output. Writes never wait for the reader. 'released' counts
// complete data eligible for delivery when the producer requests NAL batching.
class FifoOutput {
public:
   explicit FifoOutput(const char* path) : path(path) {}
   void reset();
   void discard();
   void append(const unsigned char* bytes, size_t count, bool release, bool recoveryPoint);
   size_t drain(uint32_t now);
   uint64_t dropped() const;
private:
   const char* path;
   mutable std::mutex mutex;
   ByteQueue<256 * 1024> queue;
   size_t released = 0;
   uint64_t drops = 0;
   uint32_t lastAttempt = 0;
   bool attempted = false, needRecovery = false;
   int fd = -1;
};
