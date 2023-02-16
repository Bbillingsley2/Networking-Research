#ifndef BYTE_ORDER_H
#define BYTE_ORDER_H

#include "./stdtypes.h"

//Purpose: an enum telling the endinaness
typedef enum{
  LITTLEENDIAN = 0,
  BIGENDIAN = 1,
  UNHANDLE = 2
} ENDIANNESS;
//Purpose: turn the ENDIANNESS enum to a string value thats printable
extern const char8* const NAME_ENDIANNESS[];

//Purpose: check the endinaness of the system
//Parameters: none
//Returns: an ENDIANNESS enum of the type corresponding to the system
//Notes: none
extern ENDIANNESS check_endianness(void);

//Purpose: change a network-byte ordered int to a host-byte ordered int
//Parameters: a network-byte ordered int
//Returns: a host-byte ordered integer of the same byte size
//Notes: uses check_endianness to first determine endianness of system
extern uint8 ntoh8(uint8 src);
extern uint16 ntoh16(uint16 src);
extern uint32 ntoh32(uint32 src);
extern uint64 ntoh64(uint64 src);

//Purpose: change a host-byte ordered int to a network-byte ordered int
//Parameters: a host-byte ordered int
//Returns: a network-byte ordered integer of the same byte size
//Notes: the following functions simply call their corresponding ntoh to flip
//       the process
extern uint8 hton8(uint8 src);
extern uint16 hton16(uint16 src);
extern uint32 hton32(uint32 src);
extern uint64 hton64(uint64 src);

#endif//BYTE_ORDER_H