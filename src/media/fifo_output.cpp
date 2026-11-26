#include "fifo_output.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

void FifoOutput::reset()
{
   std::lock_guard<std::mutex> lock(mutex);
   if (fd >= 0) close(fd);
   fd = -1;
   attempted = needRecovery = false;
   released = 0;
   queue.clear();
}

void FifoOutput::discard()
{
   std::lock_guard<std::mutex> lock(mutex);
   released = 0;
   queue.clear();
}

void FifoOutput::append(const unsigned char* bytes, size_t count, bool release, bool recoveryPoint)
{
   std::lock_guard<std::mutex> lock(mutex);
   if (needRecovery && !recoveryPoint) { drops += count; return; }
   needRecovery = false;
   if (!queue.append(bytes, count)) {
      drops += queue.size();
      queue.clear();
      released = 0;
      if (!recoveryPoint || !queue.append(bytes, count)) {
         drops += count;
         needRecovery = true;
         return;
      }
   }
   if (release) released = queue.size();
}

size_t FifoOutput::drain(uint32_t now)
{
   std::lock_guard<std::mutex> lock(mutex);
   if (!released) return 0;
   if (fd < 0) {
      if (attempted && now - lastAttempt < 100) return 0;
      attempted = true;
      lastAttempt = now;
      fd = open(path, O_WRONLY | O_NONBLOCK | O_CLOEXEC);
      if (fd < 0) return 0;
      fcntl(fd, F_SETPIPE_SZ, 250000);
   }
   ssize_t sent = write(fd, queue.front(), std::min(released, queue.contiguous()));
   if (sent > 0) {
      queue.consume(sent);
      released -= sent;
      return sent;
   }
   if (sent < 0 && errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
      close(fd);
      fd = -1;
      drops += queue.size();
      queue.clear();
      released = 0;
      needRecovery = true;
   }
   return 0;
}

uint64_t FifoOutput::dropped() const
{
   std::lock_guard<std::mutex> lock(mutex);
   return drops;
}
