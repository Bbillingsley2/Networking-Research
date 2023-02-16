#include "../hdr/socket_msg_help.h"
#include "../hdr/socket_help.h"
#include "../hdr/cobs.h"
#include "../hdr/byte_order.h"
#include "../hdr/parity.h"
#include "../hdr/byte_help.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <syslog.h>
#include <assert.h>

// #define DEBUG4

int msg_to_cobs(msg_t* src, uchar8 **buf, uint64 *len) {
    uint64 l2 = 0;
    uchar8* tmpbuf = NULL;
    uchar8* encoded_buf = NULL;

    assert(buf != 0);
    assert(src != 0);
    assert(len != 0);

    msg_to_bytes(src,&tmpbuf,&l2);
#ifdef DEBUG4
    (void) fprintf(stderr,"msg_to_cobs: input bytes %lu ",l2);
#endif
    if ((encoded_buf = malloc(2*l2)) == 0){	// max cobs size is far less
	return(1);
    }

    cobs_encode(tmpbuf,l2,encoded_buf);
    *buf = encoded_buf;
    *len = cobs_encoded_length_from_encoded(*buf);
#ifdef DEBUG4
    (void) fprintf(stderr,"msg_to_cobs: output bytes %lu\n",*len);
#endif
    return(0);
}

void send_msg(int sock, msg_t* src){
  uint64 size = 0;
  uchar8* bytes = NULL;

  if (msg_to_cobs(src,&bytes,&size) != 0) {
    (void) fprintf(stderr,"internal cobs failure\n");
    return;
  }
  if (writen(sock, bytes, (ssize_t)size) != size) {
#ifdef DEBUG
    (void)fprintf(stderr,"writen/send failed\n");
#endif
  } else {
#ifdef DEBUG
    fprintf(stderr, "sent message of %lu bytes\n",size);
#endif
  }

  (void) free(bytes);
}

msg_t* recv_msg(int sock){
  printf("%d", sock);
  return NULL;
  /*
  uint64 bytesRead = 0;
  uchar8* tempBytes = NULL;
  uint64 tempInt = 0;
  uint64 hsize = calculate_header_length();
  uchar8* hbytes = malloc(hsize); // check
  uint64 bsize = 0;
  uchar8* bbytes = NULL;
  uint64 tsize = calculate_trailer_length();
  uchar8* tbytes = malloc(tsize); // check
  // header
  // worse case and best case is the same (fixed lengh for header > 255)
  tempInt = cobs_encoded_length_worse_case(hsize);
  tempBytes = malloc(tempInt); // check
  bytesRead = readn(sock, tempBytes, tempInt);
  if(bytesRead<tempInt){
      if(bytesRead != 0){
          syslog(LOG_ALERT, "hipERROR: Could not Read Full Header, Missing Bytes.");
      }
      free(tempBytes);
      free(hbytes);
      free(tbytes);
      return NULL;
  }
  if(cobs_verify(tempBytes, tempInt) != true){
      syslog(LOG_ALERT, "hipERROR: Header COBS damaged.");
      free(tempBytes);
      free(hbytes);
      free(tbytes);
      return NULL;
  }
  cobs_decode(tempBytes, tempInt, hbytes);
  free(tempBytes);
  // make header here
  // use header after
  // body
  uint64 currBodySize = 0;
  memcpy(&bsize, hbytes+26, sizeof(uint32)); // 26 is where the bodyLength is
  bsize = ntoh32(bsize);
  bbytes = malloc(bsize); // check
  tempInt = cobs_encoded_length_worse_case(bsize);
  tempBytes = malloc(tempInt); // check
  bytesRead = readn(sock, tempBytes, cobs_encoded_length_best_case(bsize));
  if(bytesRead<cobs_encoded_length_best_case(bsize)){
      free(tempBytes);
      free(hbytes);
      free(tbytes);
      free(bbytes);
      return NULL;
  }
  currBodySize += cobs_encoded_length_best_case(bsize);
  while(tempBytes[currBodySize-1] != SPECIAL_CHAR){
      bytesRead += readn(sock, tempBytes+currBodySize, 1);
      if(bytesRead<1){
          syslog(LOG_ALERT, "hipERROR: Could not Read Full Body, Missing Bytes.");
          free(tempBytes);
          free(hbytes);
          free(tbytes);
          free(bbytes);
          return NULL;
      }
      currBodySize++;
  }
  if(cobs_verify(tempBytes, currBodySize) != true){
      syslog(LOG_ALERT, "hipERROR: Body COBS damaged.");
      free(tempBytes);
      free(hbytes);
      free(tbytes);
      free(bbytes);
      return NULL;
  }
  cobs_decode(tempBytes, currBodySize, bbytes);
  free(tempBytes);
  // trailer
  // best case and worse case is the same
  tempInt = cobs_encoded_length_worse_case(tsize);
  tempBytes = malloc(tempInt); // check
  bytesRead = readn(sock, tempBytes, tempInt);
  if(bytesRead<tempInt){
      syslog(LOG_ALERT, "hipERROR: Could not Read Trailer, Missing Bytes.");
      free(tempBytes);
      free(hbytes);
      free(tbytes);
      free(bbytes);
      return NULL;
  }
  if(cobs_verify(tempBytes, tempInt) != true){
      syslog(LOG_ALERT, "hipERROR: Trailer COBS damaged.");
      free(tempBytes);
      free(hbytes);
      free(tbytes);
      free(bbytes);
      return NULL;
  }
  cobs_decode(tempBytes, tempInt, tbytes);
  free(tempBytes);
  //form
#ifdef DEBUG
  printf("Received COBS-encoded Message\n");
#endif
  msg_t* res = _msg_from_bytes(hbytes, bbytes, tbytes);
  free(hbytes);
  free(bbytes);
  free(tbytes);
  return res; */
}
