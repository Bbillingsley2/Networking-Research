#include "../hdr/byte_order.h"

const char8* const NAME_ENDIANNESS[] = {[LITTLEENDIAN]="Little Endian",[BIGENDIAN]="Big Endian",[UNHANDLE]="Unknown Endian"};
ENDIANNESS check_endianness(void){
  volatile uint32 end = 0x00000001;
  return (*(volatile char8 *) &end == 0x01) ? LITTLEENDIAN : BIGENDIAN;
}

union bohelper64{ uchar8 byte[8]; uint64 quad; };
union bohelper32{ uchar8 byte[4]; uint32 quad; };
union bohelper16{ uchar8 byte[2]; uint16 quad; };

uint8 ntoh8(uint8 src){
  return src;
}
uint16 ntoh16(uint16 src){
  if(check_endianness() == LITTLEENDIAN){
    union bohelper16 tmp;
    tmp.quad = src;
    uint16 dst = 0;
    for(uint8 i = 0; i < 2; ++i)
      dst = (uint16)( (dst << 8) + tmp.byte[i] );
    return dst;
  }else{
    return src;
  }
}
uint32 ntoh32(uint32 src){
  if(check_endianness() == LITTLEENDIAN){
    union bohelper32 tmp;
    tmp.quad = src;
    uint32 dst = 0;
    for(uint8 i = 0; i < 4; ++i)
      dst = (dst << 8) + tmp.byte[i];
    return dst;
  }else{
    return src;
  }
}
uint64 ntoh64(uint64 src){
  if(check_endianness() == LITTLEENDIAN){
    union bohelper64 tmp;
    tmp.quad = src;
    uint64 dst = 0;
    for(uint8 i = 0; i < 8; ++i)
      dst = (dst << 8) + tmp.byte[i];
    return dst;
  }else{
    return src;
  }
}

uint8 hton8(uint8 src){ return ntoh8(src); }
uint16 hton16(uint16 src){ return ntoh16(src); }
uint32 hton32(uint32 src){ return ntoh32(src); }
uint64 hton64(uint64 src){ return ntoh64(src); }