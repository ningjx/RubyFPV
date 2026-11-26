#include "../../src/core/base.h"
#include "../../src/protocol/radiopackets_short.h"
#include <assert.h>

int main(void)
{
   // Independent CRC-8 test vector, initial 0xff, polynomial 0x31.
   u8 packet[] = {0xaa, 0, 0x28, 0x19, 3, 0x20, 0x30, 0x41};
   u8 crc = 0xff;
   for (unsigned i = 2; i < sizeof(packet); ++i) {
      crc ^= packet[i];
      for (int bit = 0; bit < 8; ++bit)
         crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : crc << 1;
   }
   packet[1] = crc;
   assert(sizeof(t_packet_header_short) == 5);
   assert(radio_buffer_is_valid_short_packet(packet, sizeof(packet)));
   assert(!radio_buffer_is_valid_short_packet(packet, sizeof(packet) - 1));
   packet[7] ^= 1;
   assert(!radio_buffer_is_valid_short_packet(packet, sizeof(packet)));
   radio_packets_short_init();
   for (unsigned i = 0; i <= 256; ++i)
      assert(radio_packets_short_get_next_id_for_radio_interface(0) == (u8)i);
   assert(radio_packets_short_get_next_id_for_radio_interface(1) == 0);
   return 0;
}
