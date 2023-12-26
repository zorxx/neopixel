/* \copyright 2023 Zorxx Software. All rights reserved.
 * \license This file is released under the MIT License. See the LICENSE file for details.
 * \brief ESP32 Neopixel Driver Lookup Table Generator Application
 * 
 * This application can be built under Linux with gcc via the following command:
 *   gcc -o tablegen tablegen.c
 */
#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>

#define P(...) fprintf(stderr, __VA_ARGS__)

int main(void)
{
   for(int i = 0; i < 256; ++i)
   {
      uint32_t v = 0x924924;
      for(int r = 0; r < 8; ++r)
      {
         if(i & 1 << r)
            v |= 0x1 << (1 + 3*r);
      }
      uint32_t c = htonl(v << 8);
      uint8_t *b = (uint8_t *)&c;
      P("   { 0x%02x, 0x%02x, 0x%02x }, /* %u */\n", b[0], b[1], b[2], i);
   }
   return 0; 
}
