#include "../hdr/byte_help.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_hex(uchar8* src, uint64 length){
  for(uint64 i=0; i<length; i++){
    printf("%02x ", src[i]);
  }
  printf("\n");
}

void print_bytes(uchar8* src, uint64 length){
  for(uint64 i=0; i<length; i++){
    printf("%c ", src[i]);
  }
  printf("\n");
}

void print_string(uchar8* src, uint64 length){
  for(uint64 i=0; i<length; i++){
    printf("%c", src[i]);
  }
}

void* copy_void(void* bytes, uint64 length){
  void* buffer = malloc(length);
  return memcpy(buffer,bytes,length);
}

void* combine_voids(void* b1, uint64 s1, void* b2, uint64 s2){
  void* buffer = malloc(s1 + s2);
  if(s1 != 0){memcpy(buffer,b1,s1);}
  if(s2 != 0){memcpy(((uchar8*)buffer+s1),b2,s2);}
  return buffer;
}

void* add_to_void(void* b1, uint64 s1, uint64 s2){
  void* buffer = malloc(s1 + s2);
  if(s1 != 0){memcpy(buffer,b1,s1);}
  return buffer;
}

#ifdef undef
// this seeks to estimate lengths if we pad data but we never pad
uint64 compute_buff_length(uint64 numBytes, uint64 byteChunk){
  return ( numBytes + ( (numBytes%byteChunk)==0 ? 0 : (byteChunk-(numBytes%byteChunk)) ) );
}
#endif


bool8 compare_bytes(uchar8* b1, uint64 s1, uchar8* b2, uint64 s2){
  if(s1 != s2){ return false; }
  for(uint64 i=0; i<s1; i++){
    if(b1[i] != b2[i]){
      return false;
    }
  }
  return true;
}
