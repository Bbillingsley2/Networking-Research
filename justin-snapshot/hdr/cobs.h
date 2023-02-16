#ifndef COBS_H
#define COBS_H

#include "./stdtypes.h"

//Notes: while the special char is appended to the encoded string, it itself is
//       not a 'marker' i.e. it does not translate to a corresponding normal
//       value and is only used as a delimiter. Character that you wish to
//       remove from the string (replacing it with markers).
#define SPECIAL_CHAR 0x00

//Purpose: cobs encode a normal byte array
//Parameters: a byte array pointer with its length and an already allocated
//            byte array to place the encoded byte array
//Returns: none
//Notes: the last byte is always the special char, include string delimiter
//TODOS: return length and change how it encodes
extern void cobs_encode(uchar8* src, uint64 len, uchar8* dst);

//Purpose: cobs decode an encoded byte array
//Parameters: an encoded byte array pointer with its length and an already
//            allocated byte array to place the normal byte array
//Returns: none
//Notes: good to verify before decoding, includes string delimiter
extern void cobs_decode(uchar8* src, uint64 len, uchar8* dst);

//Purpose: verify the markers and ending of a encoded byte array
//Parameters: an encoded byte array pointer with its length
//Returns: a bool value (true if correct/valid, false if wrong/corrupted)
//Notes: none
extern bool8 alt_cobs_verify(uchar8* src, uint64 len);
extern bool8 cobs_verify(uchar8* src, uint64 len);

//Purpose: get the length of a given encoded byte array
//Parameters: an encoded byte array
//Returns: the length of that encoded byte array
//Notes: for this to work, the encoded byte array must be valid
extern uint64 cobs_encoded_length_from_encoded(uchar8* src);

//Purpose: calculate the encoded length from a byte array
//Parameters: a byte array and its length
//Returns: the length of the byte array if you encode it
//Notes: none
extern uint64 cobs_encoded_length_from_decoded(uchar8* src, uint64 len);

//Purpose: calculate the decoded length from an encoded byte array
//Parameters: the encoded byte array
//Returns: the length of the byte array if you decode it
//Notes: the length is no needed and is calculated internally
extern uint64 cobs_decoded_length_from_encoded(uchar8* src);

//Purpose: calculate the worse case (encode length) for a given length
//Parameters:the given length to get the worse case
//Returns: the worse case for encoding a array that size
//Notes: simple calculation
extern uint64 cobs_encoded_length_worse_case(uint64 len);

//Purpose: calculate the best case (encode length) for a given length
//Parameters:the given length to get the best case
//Returns: the best case for encoding a array that size
//Notes: simple calculation
extern uint64 cobs_encoded_length_best_case(uint64 len);

// try to grab valid data from start of cobs string 
extern int cobs_partial_decode(uchar8* src, uint64 slen, uchar8* dst, uint64 dlen);

#endif//COBS_H
