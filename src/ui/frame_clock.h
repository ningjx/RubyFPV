#pragma once
#include <stdint.h>
#include <algorithm>

// Timestamps are unsigned monotonic milliseconds, including across wraparound.
class FrameClock {
public:
   bool due(uint32_t now, int fps) const {
      unsigned interval = 1000U / (fps > 0 && fps <= 1000 ? fps : 15);
      return !started || now - lastStart >= interval;
   }
   void begin(uint32_t now) {
      if (!started) sampleStart = now;
      started = true;
      lastStart = now;
   }
   void finish(uint32_t now, bool displayed) {
      if (displayed) {
         if (submitted && sampleCount < 128) intervals[sampleCount++] = now - lastDisplay;
         lastDisplay = now;
         ++inWindow;
         ++submitted;
      }
      else ++failed;
      uint32_t elapsed = now - sampleStart;
      if (elapsed >= 1000) {
         fps = static_cast<unsigned>(1000ULL * inWindow / elapsed);
         if (sampleCount) {
            std::sort(intervals, intervals + sampleCount);
            p50 = intervals[(sampleCount - 1) / 2];
            p95 = intervals[(95 * sampleCount + 99) / 100 - 1];
            sampleCount = 0;
         }
         inWindow = 0;
         sampleStart = now;
      }
   }
   unsigned fps = 0;
   uint32_t submitted = 0, failed = 0;
   uint32_t p50 = 0, p95 = 0;
private:
   uint32_t intervals[128]{}, lastDisplay = 0;
   unsigned sampleCount = 0;
   bool started = false;
   uint32_t lastStart = 0, sampleStart = 0, inWindow = 0;
};
