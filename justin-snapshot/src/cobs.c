#include "../hdr/cobs.h"
#include <stddef.h>
#include <stdio.h>
#include <assert.h>

#define PlaceMarker(X)  (*curr_dst = ((X)==SPECIAL_CHAR) ? 0x00 : (X), curr_dst = dst++, marker = 0x01)

void cobs_encode(uchar8* src, uint64 len, uchar8* dst){
  uchar8* end = src + len; // end of the byte array
  uchar8* curr_dst = dst++; // pointer to next dst value
  uchar8 marker = 0x01; // how far away the last marker is

  // loop through src byte array (normal)
  while(src < end){
    // replace the spcl chars
    if(*src == SPECIAL_CHAR){
      PlaceMarker(marker);
    }else{
      *dst++ = *src;
      marker++;
      // if the marker is to its max value, finish
      if(marker == 0xFF){ PlaceMarker(marker); }
    }
    src++;
  }
  if (marker != 0x1) {
    PlaceMarker(marker);
  }
  *curr_dst = SPECIAL_CHAR; // last char is always the special char
}

void cobs_decode(uchar8* src, uint64 len, uchar8* dst){
  uchar8* end = src + len ; // end of the byte array (not counting the spcl char)
  int marker; // the current marker
  // loop through src byte array (encoded)
  while(src < end){
    // account for if the spcl char is not 0x00
    marker = (*src == 0x00) ? SPECIAL_CHAR : *src;
    src++;
    // move the amount of spaces that the marker says to move
    for(int i=1; i<marker; i++){ *dst++ = *src++; }
    // substitute the marker for a spcl char in the output
    if(marker < 0xFF && src < end){ *dst++ = SPECIAL_CHAR; }
  }
}

// seek to extract up to N bytes from front of cobs encoded string
// does not deal with special marker

int cobs_partial_decode(uchar8* src, uint64 slen, uchar8* dst, uint64 dlen) {
    uint64 dlensave = dlen;
    uchar8 substringlen;
    int append;

    if (slen < dlen)
	goto fail;

    while ((slen > 0) && (dlen > 0)) {

	substringlen = *src;	// grab substring length
	if (substringlen == 0x0)	// hit a marker - error
	    goto fail;

	append = (substringlen != 0x255); // do we append 0x0?
	src++;
	slen--;
	substringlen--;

	// gotta ensure we don't run off src or dst while moving substring
	while ((slen > 0) && (dlen > 0) && (substringlen > 0)) {

	    *dst++ = *src++;
	    dlen--;substringlen--;slen--;
	}
	// here we can stop for 3 reasons, of which hiting end of dlen
	// or hitting end of substrlen are success -- end of slen is bad
	// unless also dlen == 0
	
	if ((slen == 0) && (dlen > 0))
	    goto fail;

	// OK, append -- assuming we're not at end of dlen
	if ((dlen > 0) && (append)) {
	    *dst++ = 0x0;
	    dlen--;
	}
    }

    return(dlensave);

fail:
    return(0);
}

//
// alt cobs verify - to replace cobs_verify,
//    not check that cobs encoding is reasonable
//
// BUGS: currently does NOT correctly deal with change of SPECIAL_CHAR from 0
//

bool8 alt_cobs_verify(uchar8* src, uint64 len) {
    uchar8* end = src+len;
    uchar8* cur;
    uchar8  substringlen;	/* length of COBS substring */

    // first path, confirm COBS encoding does not run off end of buffer

    cur=src;
    while(cur < end) {
	substringlen = *cur;

	/* zero length not permitted */
	if (substringlen == 0) {
		return(false);
	}

	// in COBS, for all lengths, move that many bytes ahead in src -- variation
	// is in destination where we may or may not append a 0
	cur += substringlen;
    }


    /* cur should == end */
    if (cur != end)
	return(false);

    /* now look for an embedded SPECIAL_CHAR */
    for (cur=src ;cur < end;cur++)
    {
	if (*cur == SPECIAL_CHAR)
	    return(false);
    }

    return(true);
}


bool8 cobs_verify(uchar8* src, uint64 len){
  //uchar8* end = src + len - 1; // end of the byte array (not counting the spcl char)
  uchar8* end = src + len; // end of the byte array (not counting the spcl char)
  uchar8 marker = 0x00; // the current marker
  printf("Message size %lu\n", len);
  // loop through src byte array (encoded)
  while(src < end){
    // there should be no spcl chars in the byte array
    if(*src == SPECIAL_CHAR){ printf("Special char in byte array\n"); return false; }
    // if the marker tells us this current char is the next marker
    if(marker == 0x00){ marker = (*src == 0x00) ? SPECIAL_CHAR : *src; }
    src++;
    marker--;
  }
  // if the marker does not tell where the delimiter is
  if(marker != 0x00){ printf("Cannot find delimited\n"); return false; }
  // if the final char is not the spcl char
  //return ( (*src == SPECIAL_CHAR) ? true : false );
  return true;
}

uint64 cobs_encoded_length_from_encoded(uchar8* src){
  uint64 i = 0;
  // the last char is always the spcl char
  while(*src != SPECIAL_CHAR){ i++; src++; }
  return ++i;
}

uint64 cobs_encoded_length_from_decoded(uchar8* src, uint64 len){
  uint64 sum = len+2; // the length of the encoded
  uint64 last_spcl_char = 0; // the last spcl char seen
  // loop through src byte array (normal)
  for(uint64 i=0; i<len; i++){
    // reset last_spcl_char if we see a spcl char
    last_spcl_char = (src[i] == SPECIAL_CHAR) ? 0 : last_spcl_char+1;
    // if we have not seen a spcl char, the encoding adds one char
    if(last_spcl_char >= 254){ sum++; last_spcl_char = 0; }
  }
  return sum;
}

// tell me how big the buffer needs to be to contain the decoded
// COBS string... Note this routine MUST receive a terminated COBS string!!

uint64 cobs_decoded_length_from_encoded(uchar8* src){
  uchar8* end;
  uint64 sum;
  uchar8  substringlen;	/* length of COBS substring */

  for(end=src; *end != SPECIAL_CHAR; end++) {
    // null body
  }

  // OK, now count
  sum = 0;

  while (src < end) {
    substringlen = *src; 

    if (substringlen == 0xFF)
	sum += 0xFE;
    else
	sum += substringlen;
    src += substringlen;
  }

  // really should check that src == end
  assert(src == end);

  return sum;
}

uint64 cobs_encoded_length_worse_case(uint64 len){ return ( len + 2 + (len/254) ); }

uint64 cobs_encoded_length_best_case(uint64 len){ return ( len + 2 ); }
