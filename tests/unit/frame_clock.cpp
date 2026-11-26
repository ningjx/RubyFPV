#include "../../src/ui/frame_clock.h"
#include <cassert>

int main()
{
   FrameClock clock;
   const uint32_t start = UINT32_MAX - 500;
   assert(clock.due(start, 15));
   for (unsigned i = 0; i < 12; ++i) {
      uint32_t now = start + i * 100;
      clock.begin(now);
      clock.finish(now, i != 3);
      assert(!clock.due(now + 65, 15));
      assert(clock.due(now + 66, 15));
   }
   assert(clock.submitted == 11 && clock.failed == 1 && clock.fps == 10);
   assert(clock.p50 == 100 && clock.p95 == 200);
   FrameClock slow;
   slow.begin(0);
   slow.finish(1500, true);
   assert(slow.fps == 0); // One successful frame in one and a half seconds.
}
