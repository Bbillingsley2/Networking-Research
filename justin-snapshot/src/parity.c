#include "../hdr/parity.h"
#include <sys/types.h>
//#include <sys/errno.h>

//extern int errno;
static int bit_table[16] = { // number of bits set in each value
	0, /*0x0*/ 1, /*0x1*/ 1, /*0x2*/ 2, /*0x3*/ 1, /*0x4*/ 2, /*0x5*/
  2, /*0x6*/ 3, /*0x7*/ 1, /*0x8*/ 2, /*0x9*/ 2, /*0xA*/ 3, /*0xB*/
  2, /*0xC*/ 3, /*0xD*/ 3, /*0xE*/ 4 /*0xF*/
};

/*
 * compute_vparity_length: get size of vertical parity (always 256 bits)
 *
 */
uint64 compute_vparity_length(void){
    return (sizeof(uint64) * 4);
}

/*
 * compute_vparity: treat buffer as a sequence of 256-bit words,
 * compute the odd parity across the n-th bit of all the words
 * args: buf[buflen] on which to compute parity
 *       parity needs to be allocated to compute_vparity_length() size
 *       note buflen need not be on 256-bit boundary
 * returns: parity is filled.
 */
void compute_vparity(uchar8* buf, uint64 buflen, uint64* parity){
    int bit_total, byteoffset, bitoffset;
    uchar8 bitmask;

    for(uint8 i=0; i<4; i++){
	parity[i] = 0;
    }

    // 256 bits
    for(uint16 i=0; i < 256; i++){
        byteoffset = i % 32;
        bitoffset = i % 8;
        bitmask = (uchar8)( 1 << bitoffset );
        bit_total = 0;

	// total up the bits in the selected position
	for(uint64 j=(uint64)byteoffset; j < buflen; j+= 32){
	    if (buf[j] & bitmask){ bit_total++; }
	}
	// only care about low bit of bit_total
	if ((bit_total & 0x1) == 0){
	   parity[i/64] |= (uint64)(1 << (i % 64));
	}
    }
}

/*
 * check_vparity: treat buffer as a sequence of 256-bit words,
 * check the given vertical parity
 * args: buf[buflen] on which to check parity
 *       parity* (already filled)
 * returns: true if good, false if bad
 */
bool8 check_vparity(uchar8* buf, uint64 buflen, uint64* parity){
    int bit_total, byteoffset, bitoffset;
    uchar8 bitmask;
    // 256 bits
    for(uint16 i=0; i < 256; i++){
	byteoffset = i % 32;
        bitoffset = i % 8;
	bitmask = (uchar8)( 1 << bitoffset );
        bit_total = 0;

        // total up the bits in the selected position
        for(uint64 j=(uint64)byteoffset; j < buflen; j+= 32){
            if(buf[j] & bitmask){ bit_total++; }
        }
	    	if(parity[i/64] & (uint64)(1 << (i % 64))){ bit_total++; }
        // only care about low bit of bit_total
        if((bit_total & 0x1) == 0){ return false; }
    }
    return true;
}


// utility routines for horizontal parity (which is variable length)
// recall treats buffer as sequence of 32 bit words

// number of words in buffer
uint64 compute_hparity_words(uint64 buflen) {
    return ( (buflen/4) + (buflen % 4 ? 1 : 0) );
}

// number of bytes of hparity data
// should be passed words but various places need to know this that don't
// need to know number of words

uint64 compute_hparity_length(uint64 bufflen){
    uint64 words = compute_hparity_words(bufflen);
    return ( (words/8) + (words % 8 ? 1 : 0) );
}

/*
 * compute_hparity: treat buffer as a sequence of 32-bit words
 * compute the odd parity across each word
 * args: buf[buflen] on which to compute parity
 *       parity needs to be allocated to compute_hparity_length() size
 *       buflen need not be aligned to 32-bit boundary
 * returns: parity is filled
 */
void compute_hparity(uchar8* buf, uint64 buflen, uchar8* parity){
    // how many bytes of parity do we need?
    // compute number of words in buffer, one bit per word, so divide words by 8
    int num_words;
    int parity_bytes;
    int bit_total = 0;

    num_words = compute_hparity_words(buflen);
    parity_bytes = compute_hparity_length(buflen);

    for(int i=0; i < parity_bytes; i++) {
	parity[i] = 0;
    }

    // for each word
    for(int i=0; i < num_words; i++){
	// compute parity for word
	bit_total = 0;
	for(int j=0; j < 4; j++){
	    // don't run off end of buffer
	    if((uint64)((i*4) + j) < buflen){
		bit_total += bit_table[buf[(i*4)+j]>>4]
		    + bit_table[buf[(i*4)+j] & 0xF];
	    }
	}

	if((bit_total & 0x1) == 0){
	    // set parity to 1 for this word
	    parity[i/8] |= (uchar8)(1 << (i % 8));
	}
    }
}

/*
 * check_hparity: treat buffer as a sequence of 32-bit words,
 * check the given horizonal parity
 * args: buf[buflen] on which to check parity
 *       parity (already filled)
 * returns: true if good, false if bad
 */
bool8 check_hparity(uchar8* buf, uint64 buflen, uchar8* parity){
    int num_words = (int)( (buflen/4) + (buflen % 4 ? 1 : 0) );
    //int parity_bytes = (num_words/8) + (num_words % 8 ? 1 : 0);
    int bit_total = 0;

    // for each word
    for(int i=0; i < num_words; i++){
	    // compute parity for word
	bit_total = ( parity[i/8] & (1 << (i % 8)) ) ? 1 : 0;

	for(int j=0; j < 4; j++){
	    // don't run off end of buffer
	    if((uint64)((i*4) + j) < buflen){
		    bit_total += bit_table[buf[(i*4)+j]>>4]
			+ bit_table[buf[(i*4)+j] & 0xF];
	    }
	}

	if((bit_total & 0x1)==0){ return false; }
    }
    return true;
}
