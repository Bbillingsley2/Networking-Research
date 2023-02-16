// basic unit tests for routines
//
//
//
//

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <strings.h>

#include "../hdr/byte_help.h"
#include "../hdr/byte_order.h"
#include "../hdr/cobs.h"
#include "../hdr/conn_chan.h"
#include "../hdr/message.h"
#include "../hdr/parity.h"
#include "../hdr/section.h"
#include "../hdr/server_help.h"
#include "../hdr/socket_help.h"
#include "../hdr/socket_msg_help.h"
#include "../hdr/stdtypes.h"

extern int cobs_tests();

//
// type tests
//
// mostly to make sure that the sizeof(struct) == size of individual parts
// for key parts
//

int sum_parts_mh()
{
    msg_header_t mh;
    int sum = 0;

    sum += sizeof(mh.mbz);
    sum += sizeof(mh.sendID);
    sum += sizeof(mh.recvID);
    sum += sizeof(mh.msgID);
    sum += sizeof(mh.chanType);
    sum += sizeof(mh.dataType);
    sum += sizeof(mh.totalLength);
    sum += sizeof(mh.bodyLength);
    sum += sizeof(mh.contentLength);
    sum += sizeof(mh.parityLength);
    sum += sizeof(mh.segmentNum);
    sum += sizeof(mh.crc);
    return(sum);
}

int type_tests()
{
    if (sizeof(msg_header_t) != sum_parts_mh())
    {
	(void) fprintf(stderr,"sizeof msg_header_t %ld != sum of parts %d\n",
	    sizeof(msg_header_t),sum_parts_mh());
	return(1);
    }
    if (sizeof(msg_header_t) != sizeof(msg_trailer_t))
    {
	(void) fprintf(stderr,"sizeof msg_header_t != msg_trail_t\n");
	return(1);
    }


    return(0);
}

//
// basic tests of message routines
//
//
//


// compare two message structures and confirm they are the same

int cmp_msg(msg_t *m1, msg_t *m2) {

     if (bcmp((void *)m1->head,(void *)m2->head,sizeof(msg_header_t)) != 0) {
	(void) fprintf(stderr,"unit test:cmp_msg:header\n");
	return(1);
     }
     if (bcmp((void *)m1->trail,(void *)m2->trail,sizeof(msg_trailer_t)) != 0) {
	(void) fprintf(stderr,"mbz(%d,%d)\n",(int)m1->trail->mbz,(int)m2->trail->mbz);
	(void) fprintf(stderr,"chan(%d,%d)\n",(int)m1->trail->chanType,(int)m2->trail->chanType);
	(void) fprintf(stderr,"data t(%d,%d)\n",(int)m1->trail->dataType,(int)m2->trail->dataType);
	(void) fprintf(stderr,"msgID(%u,%u)\n",m1->trail->msgID,m2->trail->msgID);
	(void) fprintf(stderr,"totLen(%u,%u)\n",m1->trail->totalLength,m2->trail->totalLength);
	(void) fprintf(stderr,"bodLen(%u,%u)\n",m1->trail->bodyLength,m2->trail->bodyLength);
	(void) fprintf(stderr,"parLen(%u,%u)\n",m1->trail->parityLength,m2->trail->parityLength);
	(void) fprintf(stderr,"sID(%lu,%lu)\n",m1->trail->sendID,m2->trail->sendID);
	(void) fprintf(stderr,"rID(%lu,%lu)\n",m1->trail->recvID,m2->trail->recvID);
	(void) fprintf(stderr,"segnum(%lu,%lu)\n",m1->trail->segmentNum,m2->trail->segmentNum);
	(void) fprintf(stderr,"crc(%lu,%lu)\n",m1->trail->crc,m2->trail->crc);
	(void) fprintf(stderr,"unit test:cmp_msg:trailer\n");
	return(1);
     }

     // know header fields are equal so just check body fields...

     if (bcmp((void *)m1->body->content,(void *)m2->body->content,m1->head->contentLength) != 0) {
	(void) fprintf(stderr,"unit test:cmp_msg:body\n");
	return(1);
     }

     if (bcmp((void *)m1->body->vparityData,(void *)m2->body->vparityData,4*sizeof(uint64)) != 0) {
	(void) fprintf(stderr,"unit test:cmp_msg:body\n");
	return(1);
     }

     // need to bcmp hparity, but not now
     //
     return(0);
}

#define DATASIZE 512
static uchar8 somedata[DATASIZE];

int msg_tests()
{
    msg_t *m, *m2;
    uchar8* msg_in_bytes;
    uint64 mib_len;

    // create some test data
    {
	int i;

	for(i=0; i < DATASIZE; i++)
	    somedata[i] = i & 0xFF;
    }

    // now make a message

    if ((m = form_msg(0,0,0,0,0,somedata,DATASIZE)) == 0) {
	(void) fprintf(stderr,"unit test: form_msg\n");
	return(1);
    }

    msg_to_bytes(m,&msg_in_bytes,&mib_len);

    (void) fprintf(stderr,"msg_to_bytes -> %lu bytes\n",mib_len);

    if (mib_len < DATASIZE) {
	(void) fprintf(stderr,"unit test: msg_to_bytes 1\n");
	return(1);
    }
     if (bcmp((void *)m->head,(void *)m->trail,sizeof(msg_header_t)) != 0) {
	(void) fprintf(stderr,"unit test: head<>trail\n");
	return(1);
     }

    if ((m2 = msg_from_bytes(msg_in_bytes,mib_len)) == 0) {
	(void) fprintf(stderr,"unit test: msg_from_bytes 2\n");
	return(1);
    }

    if (cmp_msg(m,m2) != 0) {
	(void) fprintf(stderr,"unit test: cmp_msg\n");
	return(1);
    }

    // next step is to encode m into cobs and then decode the cobs and see if we still get m
    {
	uchar8* buf;
	uchar8* buf2;
	uint64 buflen, b2len;

	msg_to_cobs(m,&buf,&buflen);

	b2len = cobs_decoded_length_from_encoded(buf);

	if (mib_len != b2len) {
	    (void) fprintf(stderr,
		"unit test:pre-encode len [%lu] <> decode len [%lu]\n",
		    mib_len,b2len);
	    return(1);
	}
	buf2 = malloc(b2len);
	cobs_decode(buf,buflen,buf2);
	m2 = msg_from_bytes(buf2,b2len);
	
	if (cmp_msg(m,m2) != 0) {
	    (void) fprintf(stderr,"unit test:cmp_msg w/ cobs\n");
	    return(1);
	}
    }

    return(0);
}



//
//
int main(int argc, char** argv)
{
    if (cobs_tests() != 0)
	goto fail;
    if (type_tests() != 0)
	goto fail;
    if (msg_tests() != 0)
	goto fail;

    (void) fprintf(stderr,"unit tests succeed\n");
    return(0);
fail:
    (void) fprintf(stderr,"unit tests failed\n");
    return(1);
}

