#pragma once
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>

// Fixed storage. The owner provides synchronization when sharing across threads.
template<size_t Capacity>
class ByteQueue {
public:
   size_t size() const { return used; }
   const unsigned char* front() const { return data.data() + head; }
   size_t contiguous() const { return std::min(used, Capacity - head); }
   bool append(const unsigned char* bytes, size_t count) {
      if (count > Capacity - used) return false;
      size_t tail = (head + used) % Capacity;
      size_t first = std::min(count, Capacity - tail);
      std::memcpy(data.data() + tail, bytes, first);
      std::memcpy(data.data(), bytes + first, count - first);
      used += count;
      return true;
   }
   void consume(size_t count) {
      head = (head + count) % Capacity;
      used -= count;
   }
   void clear() { head = used = 0; }
private:
   std::array<unsigned char, Capacity> data;
   size_t head = 0, used = 0;
};
