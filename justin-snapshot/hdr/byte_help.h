#ifndef BYTE_HELP_H
#define BYTE_HELP_H

#include "./stdtypes.h"

//Purpose: printing a byte string in hex values
//Parameters: byte array pointer and the length of that pointer
//Returns: none
//Notes: include the delimiter in pointer and length,
//       spaces out the hex values for readabiltiy
extern void print_hex(uchar8* src, uint64 length);

//Purpose: printing a byte string in byte values
//Parameters: byte array pointer and the length of that pointer
//Returns: none
//Notes: include the delimiter in pointer and length,
//       spaces out the hex values for readabiltiy
extern void print_bytes(uchar8* src, uint64 length);

//Purpose: printing a byte string in char values
//Parameters: byte array pointer and the length of that pointer
//Returns: none
//Notes: include the delimiter in pointer and length,
//       spaces out the hex values for readabiltiy
extern void print_string(uchar8* src, uint64 length);

//Purpose: make a copy of an arbitrary pointer
//Parameters: a void pointer and the length of that pointer
//Returns: a void pointer of size length that is an exact copy
//Notes: essentially just a memcpy call
extern void* copy_void(void* bytes, uint64 length);

//Purpose: make an arbitrary pointer by using two arbitrary pointers
//Parameters: a arbitrary pointer with its length and another arbitrary pointer
//            with its length
//Returns: a arbitrary pointer that has the combined length of both pointers
//Notes: this does not get rid of the starting arbitrary pointers
extern void* combine_voids(void* b1, uint64 s1, void* b2, uint64 s2);

//Purpose: simply make an arbitrary pointer longer
//Parameters: an arbitrary pointer, its length, and much longer to add
//Returns: an arbitrary pointer that is longer
//Notes: this does not get rid of the starting arbitrary pointer,
//       it also does not nullify the added bytes (could be garbage)
extern void* add_to_void(void* b1, uint64 s1, uint64 s2);

//Purpose: get a size that is rounded according to chunksize
//Parameters: a base size and a chunk size (both uint64 for compatibility)
//Returns: a buffed size
//Notes: none
extern uint64 compute_buff_length(uint64 size, uint64 chunkSize);

//Purpose: finding if two byte strings equal in all values and length
//Parameters: two byte strings and their length
//Returns: a bool value, true if equal, false if not
//Notes: include the delimiter in pointer and length
extern bool8 compare_bytes(uchar8* b1, uint64 s1, uchar8* b2, uint64 s2);

#endif//BYTE_HELP_H