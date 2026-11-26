#include "../../src/media/fifo_output.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

int main()
{
   ByteQueue<8> ring;
   const unsigned char sequence[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
   assert(ring.append(sequence, 7));
   ring.consume(4);
   assert(ring.append(sequence + 7, 3));
   assert(!ring.append(sequence, 3));
   for (unsigned i = 4; i < 10; ++i) {
      assert(*ring.front() == i);
      ring.consume(1);
   }
   char dir[] = "/tmp/ruby-output-XXXXXX";
   assert(mkdtemp(dir));
   char path[128];
   snprintf(path, sizeof(path), "%s/preview", dir);
   assert(mkfifo(path, 0600) == 0);
   signal(SIGPIPE, SIG_IGN);
   FifoOutput sink(path);
   std::vector<unsigned char> data(240000), result;
   for (size_t i = 0; i < data.size(); ++i) data[i] = static_cast<unsigned char>(i);
   sink.append(data.data(), data.size(), false, true);
   assert(sink.drain(0) == 0);
   sink.append(sequence, 1, true, false);
   assert(sink.drain(10) == 0); // No reader.
   int reader = open(path, O_RDONLY | O_NONBLOCK);
   assert(reader >= 0);
   for (unsigned tick = 110; result.size() <= data.size() && tick < 3000; ++tick) {
      sink.drain(tick);
      unsigned char chunk[4096];
      ssize_t n = read(reader, chunk, sizeof(chunk));
      if (n > 0) result.insert(result.end(), chunk, chunk + n);
   }
   data.push_back(0);
   assert(result == data && !sink.dropped());
   sink.append(data.data(), data.size(), true, true);
   sink.append(data.data(), data.size(), true, false);
   assert(sink.dropped() == 2 * data.size());
   sink.append(sequence, 2, true, false); // Suppress a predicted frame after overflow.
   assert(sink.drain(4000) == 0);
   sink.append(sequence + 4, 3, true, true);
   assert(sink.drain(4001) == 3);
   unsigned char recovered[3];
   assert(read(reader, recovered, 3) == 3 && !memcmp(recovered, sequence + 4, 3));
   close(reader);
   sink.append(sequence, 3, true, true);
   assert(sink.drain(4002) == 0 && sink.dropped() == 2 * data.size() + 5);
   sink.reset();
   unlink(path);
   rmdir(dir);
}
