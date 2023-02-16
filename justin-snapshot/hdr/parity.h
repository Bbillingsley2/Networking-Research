#ifndef PARITY_H
#define PARITY_H

#include "./stdtypes.h"

#define BUFFER_CHUNK_SIZE 32 // in bytes

//Purpose: derive the vertical parity given a buffed byte string
//Parameters: a buffed byte string and its length, and an already allocated
//            parity byte string
//Returns: none, fills the parity byte string with vertical data
//Notes: none
extern void compute_vparity(uchar8* buf, uint64 buflen, uint64* parity);

//Purpose: check based on the vertical parity if the byte string is valid
//Parameters: a buffed byte string and its length and its vertical parity
//Returns: a bool value (true for valid data, false for corrupted data)
//Notes: none
extern bool8 check_vparity(uchar8* buf, uint64 buflen, uint64* parity);

//Purpose: compute the vertical parity length
//Parameters: none
//Returns: an int representing the vertical parity length
//Notes: vertical parity length will always be the same
extern uint64 compute_vparity_length(void);


//Purpose: derive the horizonal parity given a buffed byte string
//Parameters: a buffed byte string and its length, and an already allocated
//            parity byte string
//Returns: none, fills the parity byte string with horizontal data
//Notes: none
extern void compute_hparity(uchar8* buf, uint64 buflen, uchar8* parity);

//Purpose: check based on the horizontal parity if the byte string is valid
//Parameters: a buffed byte string and its length and its horizontal parity
//Returns: a bool value (true for valid data, false for corrupted data)
//Notes: none
extern bool8 check_hparity(uchar8* buf, uint64 buflen, uchar8* parity);

//Purpose: compute the horizontal parity length
//Parameters: a buffed length to derive the horizonal parity length from
//Returns: an int representing the horizonal parity length
//Notes: none
extern uint64 compute_hparity_length(uint64 buflen);

#endif//PARITY_H