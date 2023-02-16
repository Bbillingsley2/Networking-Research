#include "../hdr/stdtypes.h"

#include <time.h>
#include <stdlib.h>

#define IMAX_BITS(m) ((m)/((m)%255+1) / 255%255*8 + 7-86/((m)%255+12))
#define RAND_MAX_WIDTH IMAX_BITS(RAND_MAX)
_Static_assert((RAND_MAX & (RAND_MAX + 1u)) == 0, "RAND_MAX not a Mersenne number");

static uint64 rand_uint64(void){
  uint64 r = 0;
  for(uint8 i=0; i<64; i++){
    r = r*2 + (uint64)rand()%2;
  }
  return r;
}

uint64 generate_random_uint(uint64 min, uint64 max){
  static bool firstTime = true;
  if(firstTime){ srand((uint32)time(NULL)); firstTime = false; }
  return (rand_uint64() % (max - min)) + min;
}