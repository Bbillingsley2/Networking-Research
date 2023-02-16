
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <syslog.h>
#include <arpa/inet.h>
#include "../hdr/message.h"
#include "../hdr/byte_order.h"

#include <assert.h>

// #define DEBUG4

// calculate various sizes
uint64 calculate_header_length(void){
  return (sizeof(msg_header_t));
}

uint64 calculate_trailer_length(void){
  return (sizeof(msg_trailer_t));
}

uint64 calculate_parity_length(uint64 bufflen){
  return compute_vparity_length() + compute_hparity_length(bufflen);
}

uint64 calculate_body_length(uint64 clen){
  return clen + calculate_parity_length(clen);
}

uint64 calculate_total_length(uint64 clen){
  return calculate_header_length() + calculate_body_length(clen) + calculate_trailer_length();
}

/* 
 * local safe copy routine
 * ensures pointer to data we copy from doesn't run off end of buffer (eob)
 */
static int safe_memcpy(const void *eob,void *dest,void *src,const unsigned len,
	char *where)
{
    if ((eob == 0) || (dest == 0) || (src == 0) || ((src+len) > eob)) {
	assert(1);	/* for now, bail */
	if ((eob == 0) || (dest == 0) || (src == 0)) {
		fprintf(stderr, "null memcpy at %s\n",where);	
	} else {
		fprintf(stderr, "overrun memcpy (len=%u, gap=%lu) at %s\n",len,eob-src,where);	
	}
	return(1);
    }
    memcpy(dest,src,len);
    //fprintf(stderr, "Source length %zu \n dest length %zu \n", malloc_usable_size(*src), malloc_usable_size(*dest));
    return(0);
}

const char8* const NAME_CHANNEL_TYPE[] = {[DATA]="Data", [CONTROL]="Control"};
const char8* const NAME_DATA_TYPE[] = {[SYN]="Synchronize",[ACK]="Acknowledge",[RTS]="Retransmission",[FIN]="Finish"};

void free_msg(msg_t* src){
  free(src->body->content);
  free(src->body->hparityData);
  free(src->body->vparityData);
  free(src->head);
  free(src->body);
  free(src->trail);
  free(src);
}
msg_t* form_msg(uint64 sID, uint64 rID, uint8 chant, uint8 datat, uint64 snum, uchar8* content, uint32 clen){
  msg_t* m = malloc(sizeof(msg_t)); // check
  bzero(m,sizeof(*m));
  m->head = malloc(sizeof(msg_header_t)); // check
  bzero(m->head,sizeof(msg_header_t));
  m->body = malloc(sizeof(msg_body_t)); // check
  m->trail = malloc(sizeof(msg_trailer_t)); // check
  bzero(m->trail,sizeof(msg_trailer_t));
  m->head->sendID = sID;
  m->head->recvID = rID;
  m->head->chanType = chant;
  m->head->dataType = datat;
  m->head->msgID = (uint32)generate_random_uint(0, 4294967295);
  m->head->contentLength = clen;
  m->head->parityLength = (uint32)calculate_parity_length(clen);
  m->head->bodyLength = (uint32)calculate_body_length(clen);
  m->head->totalLength = (uint32)calculate_total_length(clen);
  m->head->segmentNum = snum;
  m->head->crc = 0;
  m->body->content = copy_void(content, clen);
  m->body->vparityData = malloc(compute_vparity_length()); // check
  m->body->hparityData = malloc(m->head->parityLength - compute_vparity_length()); // check
  compute_vparity(content, clen, m->body->vparityData);
  compute_hparity(content, clen, m->body->hparityData);
  m->trail->sendID = m->head->sendID;
  m->trail->recvID = m->head->recvID;
  m->trail->chanType = m->head->chanType;
  m->trail->dataType = m->head->dataType;
  m->trail->msgID = m->head->msgID;
  m->trail->totalLength = m->head->totalLength;
  m->trail->bodyLength = m->head->bodyLength;
  m->trail->contentLength = m->head->contentLength;
  m->trail->parityLength = m->head->parityLength;
  m->trail->segmentNum = m->head->segmentNum;
  m->trail->crc = m->head->crc;
  return m;
}

// msg_header_to_bytes - stuff header into bytes

void msg_header_to_bytes(msg_header_t* hdr,uchar8*dst,uint64 dstlen, uint64*len) {
    msg_header_t t;

    assert(dstlen >= sizeof(t));

    t = *hdr;
    t.sendID = hton64(t.sendID);
    t.recvID = hton64(t.recvID);
    t.segmentNum = hton64(t.segmentNum);
    t.crc = hton64(t.crc);
    t.msgID = htonl(t.msgID);
    t.totalLength = htonl(t.totalLength);
    t.bodyLength = htonl(t.bodyLength);
    t.contentLength = htonl(t.contentLength);
    t.parityLength = htonl(t.parityLength);
    t.mbz = htons(t.mbz);

    bcopy((void *)&t,(void *)dst,sizeof(t));

    if (len != 0)
	*len += sizeof(t);
}

void msg_trailer_to_bytes(msg_trailer_t* t,uchar8 *dst, uint64 dstlen,uint64 *currlen) {
    assert(dstlen >= sizeof(t));
    msg_header_to_bytes((msg_header_t*)t,dst,dstlen,currlen);
}

// takes a msg_t and puts it into buffer
// returns buffer pointer in *dst and buffer length in *len
//
void msg_to_bytes(msg_t* src, uchar8** dst, uint64* len){
  // init
  uint64 currlen;
  uchar8* bytes; // will go into *dst
  uint64 t64;

  *len = calculate_total_length(src->head->contentLength);
  bytes = malloc(*len);
  memset(bytes,0,*len);

  currlen = 0; // size of buffer filled so far

#ifdef DEBUG4
    (void) fprintf(stderr,"msg_to_bytes: creating message of len %lu\n",*len);
#endif

  // header
  msg_header_to_bytes(src->head,bytes,*len,&currlen);

#ifdef DEBUG4
    (void) fprintf(stderr,"header [%lu] ",currlen);
#endif

  // body
  // content
  memcpy(bytes+currlen, src->body->content, src->head->contentLength);
#ifdef DEBUG2
  printf("msg_to_bytes:Buff Length of res-> body is %lu.\n", src->head->contentLength);
#endif
  currlen += src->head->contentLength;
  // vertical parity data
  for(uint8 i=0; i<4; i++){
    t64 = hton64(src->body->vparityData[i]);
    memcpy(bytes+currlen, &t64, sizeof(t64));
    currlen += sizeof(t64);
  }
  // horizonal parity data
  t64 = compute_hparity_length(src->head->contentLength);
  memcpy(bytes+currlen, src->body->hparityData, t64);
  currlen += t64;

#ifdef DEBUG4
    (void) fprintf(stderr,"body [%lu] ",currlen);
#endif
  // trailer
  msg_trailer_to_bytes(src->trail,bytes+currlen,currlen,&currlen);
#ifdef DEBUG4
    (void) fprintf(stderr,"trailer [%lu]\n",currlen);
#endif
  *len = currlen; // DEBUG4, try changing to actual and see what happens
  // end
  *dst = bytes;
}


// support routines for msg_from_bytes
//
//

static void print_msg_header(msg_header_t* mhp)
{
#ifdef DEBUG2
    (void) fprintf(stderr,"SID=%lu, RID=%lu, MsgID=%u, chanType=%u\n",
	mhp->sendID,mhp->recvID,mhp->msgID,(unsigned)mhp->chanType);
#endif
}


msg_body_t *msg_body_from_bytes(uchar8 *src, uint64 len, uchar8 **consumed, uint64* remain, uint64 contentlen, uint64 paritylen) {

    uchar8 *current;
    uchar8 *eos = src+len;
    msg_body_t *result = 0;
    int VPL = compute_vparity_length(); // should be constant
    int hparlen;

    // basic checks
    if ((len<=0) || (*consumed == 0) || (contentlen >= MAX_MESSAGE_SIZE))
	goto fail;

    // now real work starts
    current = src;
    if ((result = malloc(sizeof(*result))) == 0)
	goto fail;
    bzero((void *)result,sizeof(*result)); // so fail can safely free

    // content
    if ((result->content = malloc(contentlen)) == 0) {
	goto fail;
    }

    if (safe_memcpy(eos,result->content,src,contentlen,"body_from_bytes:a") != 0) {
	goto fail;
    }
    current += contentlen;

    // vertical parity
    if ((result->vparityData = malloc(VPL)) == 0)
	goto fail;

    if (safe_memcpy(eos,result->vparityData,current,VPL,"body_from_bytes:b") != 0)
	goto fail;
    current += VPL;

    { // flip parity bits
	int i;
	for(i=0; i < 4; i++)
	    result->vparityData[i] = ntoh64(result->vparityData[i]);
    }

    // horizontal parity length
    hparlen = paritylen - VPL;

    if ((result->hparityData = malloc(hparlen)) == 0)
	goto fail;

    if (safe_memcpy(eos,result->hparityData, current, hparlen, "body_from_bytes:c") != 0)
	goto fail;
    current += hparlen;
  
    *consumed = current;
    *remain = len - (current - src);
#ifdef DEBUG4
    (void) fprintf(stderr,"msg_body_from_bytes [%lu leaving %lu] \n",(current-src),*remain);
#endif
    return(result);

fail:
#ifdef DEBUG2
    (void) fprintf(stderr,"msg_body_from_bytes:body parse failed!\n");
#endif
    if (result != 0) {
	if (result->content != 0) (void) free(result->content);
	if (result->vparityData != 0) (void) free(result->vparityData);
	if (result->hparityData != 0) (void) free(result->hparityData);
	(void) free(result);
    }
    return((msg_body_t *)0);
}

static msg_header_t *msg_ht_from_bytes(uchar8 *src, uint64 len, uchar8 **consumed,
		uint64* remain, char *diag) {
    uchar8 *current;
    uchar8 *eos = src+len;
    msg_header_t *result = 0;

    // basic checks
    assert(consumed!=0);
    assert(src!=0);
    assert(len>=0);
    assert(*consumed!=0);

    // now real work starts
    current = src;
    if ((result = malloc(sizeof(*result))) == 0)
	goto fail;

    bzero(result,sizeof(*result));

    if (safe_memcpy(eos,result,current,sizeof(*result),diag) != 0)
	goto fail;

    result->sendID = ntoh64(result->sendID);
    result->recvID = ntoh64(result->recvID);
    result->segmentNum = ntoh64(result->segmentNum);
    result->crc = ntoh64(result->crc);
    result->msgID = ntohl(result->msgID);
    result->totalLength = ntohl(result->totalLength);
    result->bodyLength = ntohl(result->bodyLength);
    result->contentLength = ntohl(result->contentLength);
    result->parityLength = ntohl(result->parityLength);
    result->mbz = ntohs(result->mbz);

    *consumed = current+sizeof(*result);
    *remain = len - sizeof(*result);

#ifdef DEBUG4
    fprintf(stderr,"%s [%lu bytes leaving %lu]\n",diag,sizeof(*result),*remain);
#endif
    return(result);

fail:
#ifdef DEBUG3
    (void) fprintf(stderr,"msg_header_from_bytes(%ux,%ld,%ux,%x):parse (%d) failed!\n",
	(unsigned)src, len, (unsigned)consumed, (unsigned)remain,sizeof(msg_header_t));
#endif
    if (result != 0) {
	(void) free(result);
    }
    return((msg_header_t *)0);
}

msg_header_t *msg_header_from_bytes(uchar8 *src, uint64 len, uchar8 **consumed, uint64* remain) {
    return(msg_ht_from_bytes(src,len,consumed,remain,"header:a"));
}

msg_trailer_t *msg_trailer_from_bytes(uchar8 *src, uint64 len, uchar8 **consumed, uint64* remain) {
    return((msg_trailer_t *)msg_ht_from_bytes(src,len,consumed,remain,"trailer:a"));
}





// buffer src contains len decoded bytes which we now convert to a msg_t while
// also error checking.
// src is guaranteed to be aligned
//

msg_t* msg_from_bytes(uchar8* src, uint64 len){
  msg_t* res = malloc(sizeof(msg_t)); // check

  (void) bzero(res,sizeof(*res));

#ifdef DEBUG4
    fprintf(stderr, "msg_from_bytes:message of length %lu is being converted from bytes\n", len);
#endif

  if ((res->head = msg_header_from_bytes(src,len,&src,&len)) == 0)
    return((msg_t *)0);

  if ((res->body = msg_body_from_bytes(src,len,&src,&len,
      res->head->contentLength,res->head->parityLength)) == 0)
      goto fail;

  if ((res->trail = msg_trailer_from_bytes(src,len,&src,&len)) == 0)
    goto fail;

  return res;

fail:
    if (res->head != 0)
	(void) free(res->head);
    if (res->body != 0)
	(void) free(res->body);
    (void) free(res);
    return((msg_t *)0);
}

void print_msg(msg_t* src, bool8 printContent, bool8 printvParity, bool8 printhParity){
#ifdef NOTDEF
  // check for null allocations in message, assertion
  // messages that are broken (print parts of the message, header trailer body)
  printf("Message\n");
  printf("-------\n");
  printf("header:\n");
  printf("   | (ID %lu) to (ID %lu)\n", src->head->sendID, src->head->recvID);
  printf("   | %s | %s\n", NAME_DATA_TYPE[src->head->dataType], NAME_CHANNEL_TYPE[src->head->chanType]);
  printf("   | segment %lu\n", src->head->segmentNum);
  printf("   | lengths:\n");
  printf("   |   total:%u, body:%u, content:%u, parity:%u\n", src->head->totalLength, src->head->bodyLength, src->head->contentLength, src->head->parityLength);
  printf("   | ID: %u\n", src->head->msgID);
  printf("   | CRC: %lu\n", src->head->crc);
  printf("body:\n");
  if(printContent){
      printf("   | content: ");
      print_hex(src->body->content, src->head->contentLength);
  }else{ printf("   | content\n"); }
  if(printvParity){
      printf("   | vparity: (%llu, %llu, %llu, %llu)\n", src->body->vparityData[0], src->body->vparityData[1], src->body->vparityData[2], src->body->vparityData[3]);
  }else{ printf("   | vparity\n"); }
  if(printhParity){
      printf("   | hparity: ");
      print_hex(src->body->hparityData, (src->head->parityLength - compute_vparity_length()));
  }else{ printf("   | hparity\n"); }
  printf("trailer:\n");
  printf("   | (ID %llu) to (ID %llu)\n", src->trail->sendID, src->trail->recvID);
  printf("   | %s | %s\n", NAME_DATA_TYPE[src->trail->dataType], NAME_CHANNEL_TYPE[src->trail->chanType]);
  printf("   | segment %lu\n", src->trail->segmentNum);
  printf("   | lengths:\n");
  printf("   |   total:%u, body:%u, content:%u, parity:%u\n", src->trail->totalLength, src->trail->bodyLength, src->trail->contentLength, src->trail->parityLength);
  printf("   | ID: %u\n", src->trail->msgID);
  printf("   | CRC: %lu\n", src->trail->crc);
#endif
}

bool8 verify_msg(msg_t* src){
  // header+trailer data
  if(src->head->sendID != src->trail->sendID){
    syslog(LOG_ALERT, "hipERROR: SendID (8 bytes) Mismatch within a Packet.");
    return false;
  }
  if(src->head->recvID != src->trail->recvID){
    syslog(LOG_ALERT, "hipERROR: RecvID (8 bytes) Mismatch within a Packet.");
    return false;
  }
  if(src->head->msgID != src->trail->msgID){
    syslog(LOG_ALERT, "hipERROR: MsgID (4 bytes) Mismatch within a Packet.");
    return false;
  }
  if(src->head->chanType != src->trail->chanType){
    syslog(LOG_ALERT, "hipERROR: ChanType (1 bytes) Mismatch within a Packet.");
    return false;
  }
  if(src->head->dataType != src->trail->dataType){
    syslog(LOG_ALERT, "hipERROR: DataType (1 bytes) Mismatch within a Packet.");
    return false;
  }
  if(src->head->totalLength != src->trail->totalLength){
    syslog(LOG_ALERT, "hipERROR: Total Length (4 bytes) Mismatch within a Packet.");
    return false;
  }
  if(src->head->bodyLength != src->trail->bodyLength){
    syslog(LOG_ALERT, "hipERROR: Body Length (4 bytes) Mismatch within a Packet.");
    return false;
  }
  if(src->head->contentLength != src->trail->contentLength){
    syslog(LOG_ALERT, "hipERROR: Content Length (4 bytes) Mismatch within a Packet.");
    return false;
  }
  if(src->head->parityLength != src->trail->parityLength){
    syslog(LOG_ALERT, "hipERROR: Parity Length (4 bytes) Mismatch within a Packet.");
    return false;
  }
  if(src->head->segmentNum != src->trail->segmentNum){
    syslog(LOG_ALERT, "hipERROR: Segment Number (8 bytes) Mismatch within a Packet.");
    return false;
  }
  if(src->head->crc != src->trail->crc){
    syslog(LOG_ALERT, "hipERROR: CRC (8 bytes) Mismatch within a Packet.");
    return false;
  }
  printf("Header and trailer CORRUPTED\n");
  // lengths of message are correct
  if(src->head->totalLength != calculate_total_length(src->head->contentLength)){
    syslog(LOG_ALERT, "hipERROR: Total Length and Content Length does not link up within a Packet.");
    return false;
  }
  uint32 tempsize = (uint32)( calculate_header_length() + src->head->bodyLength + calculate_trailer_length() );
  if(src->head->totalLength != tempsize){
    syslog(LOG_ALERT, "hipERROR: Total Length and Body Length does not link up within a Packet.");
    return false;
  }
  tempsize = (uint32)src->head->contentLength;
  if(src->head->parityLength != calculate_parity_length(tempsize)){
    syslog(LOG_ALERT, "hipERROR: Parity Length and Content Length does not link up within a Packet.");
    return false;
  }
  printf("Lengths CORRUPTED\n");
  // vparity data
  if(check_vparity(src->body->content, src->head->contentLength, src->body->vparityData) != true){
    syslog(LOG_ALERT, "hipERROR: Vertical Parity hints at an Error within a Packet.");
    return false;
  }
  // hparity data
  if(check_hparity(src->body->content, src->head->contentLength, src->body->hparityData) != true){
    syslog(LOG_ALERT, "hipERROR: Horizontal Parity hints at an Error within a Packet.");
    return false;
  }
  printf("Parity CORRUPTED\n");
  // crc
  if(src->head->crc != 0){
    syslog(LOG_ALERT, "hipERROR: CRC is messed up within a Packet.");
    return false;
  }
  return true;
}
