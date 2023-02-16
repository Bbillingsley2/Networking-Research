/*
 * file intended to have all client error parsing ...
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <assert.h>

#include "../server/hipft.h"

#include "../justin-snapshot/hdr/byte_help.h"
#include "../justin-snapshot/hdr/byte_order.h"
#include "../justin-snapshot/hdr/cobs.h"
#include "../justin-snapshot/hdr/conn_chan.h"
#include "../justin-snapshot/hdr/message.h"
#include "../justin-snapshot/hdr/parity.h"
#include "../justin-snapshot/hdr/section.h"
#include "../justin-snapshot/hdr/server_help.h"
#include "../justin-snapshot/hdr/socket_help.h"
#include "../justin-snapshot/hdr/socket_msg_help.h"
#include "../justin-snapshot/hdr/stdtypes.h"
#include "../justin-snapshot/hdr/file_handle.h"

#include "handle_errors.h"
#include "error_util.h"

/*
 * prints to stdout (which needs to be redirected in init....)
 * uses CSV format:
     * peer IP, last valid block received, type of error, subtype of error, error specific data
 *
 * type of error currently limited to: cobs, msg
 */


static void log_error(struct sockaddr_in *peer, long long bcount,
	char *type, char *subtype, char *specific, uchar8 *pkt, int pktlen)
{
    (void) fprintf(stderr, "%s,%lld,",inet_ntoa(peer->sin_addr),bcount);
    (void) fprintf(stderr, "%s,",type);

    // subtype and specific
    //
    (void) fprintf(stderr,"%s,%s,",(subtype==0?"unknown":subtype),
			(specific==0?"":specific));

    // now if provided packet data, print it
    if ((pkt != 0) && (pktlen > 0))
    {
	register int i;

	for(i=0; i < pktlen; i++)
	{
	    (void) fprintf(stderr,"%.2x",(unsigned)(pkt[i]));
	}
    }
    (void) fprintf(stderr,",");

    (void) fprintf(stderr,"\n");
}

/*
 * null packet - a version of likely COBS error
 */

void handle_null_packet(struct sockaddr_in *peer,long long blockcount)
{
    log_error(peer,blockcount,"cobs","null packet",(char *)0,(uchar8 *)0,0);
}

//
// cobs_errors
//
// handle_cobs_errors() and its subroutines
//
// called if there's a COBS decoding problem
//
// multiple possibilities
// 1. one of the COBS lengths was trashed in the message -- causing COBS to
// run off the end of the buffer, but much of the packet may be valid
// 2. a delimiter was written into the middle...
//

// hce_try_header tries to grab a header from the busted COBS
// and figure out what went wrong...
// returns 0 if success -- 1 if try something else
//

int hce_try_header(struct sockaddr_in *peer, long long blockcount,
    uchar8 *cobs,int len)
{
    char tmp[128];
    msg_header_t mh;

    if (cobs_partial_decode(cobs,len,(uchar8 *)&mh,sizeof(mh)) != sizeof(mh))
	return(1);

    if (ntohl(mh.totalLength) > len)
    {
	// I have a partial packet or header content is trashed or both
	(void) snprintf(tmp,sizeof(tmp)-1,"msg len(%d)>buffer len(%d)",
	    ntohl(mh.totalLength),len);
	log_error(peer,blockcount,"cobs","fragment-header",tmp,(uchar8 *)0,0);
	return(0);
    }

    // now it gets hard and out of time to fix today
    log_error(peer,blockcount,"cobs","smashed packet",tmp,(uchar8 *)0,0);
    return(0);
}

void handle_cobs_errors(struct sockaddr_in *peer,long long blockcount,
    uchar8 *cobs_string,int len)
{
    char tmp[128];

    // got a useful header and diagnosed...
    if (hce_try_header(peer,blockcount,cobs_string,len) == 0)
	return;

    // at this point, header either trashed or not present
    // need to try for trailer

    // declare unknown cobs error
    (void) snprintf(tmp,sizeof(tmp)-1,"len=%d",len);
    log_error(peer,blockcount,"cobs","unknown",tmp,(uchar8 *)0,0);
}

// ***********************************************************************
// ***********************************************************************
// code that understands messages

// handle_busted_msg1
//
// we couldn't extract a parsed (but not evaluated) message from this block
// of bytes
// 
// COBS successfully decoded, so any errors that did occur were not in the
// COBS length bytes.
// 
// we have the following choices
// 1. this message is a runt message but valid COBS
// 2. some element of the content between COBS length bytes was trashed
//    in a way that inhibits parsing

void handle_busted_msg1(struct sockaddr_in *peer,long long blockcount,uchar8 *buffer,
    uint64 len)
{
    char tmpbuf[128];
    uchar8 *save_buf = buffer;
    int save_len = len;
    msg_t* res;

    res = malloc(sizeof(msg_t));
    assert(res != 0);
    (void) bzero(res,sizeof(*res));


    if ((res->head = msg_header_from_bytes(buffer,len,&buffer,&len)) == 0)
    {
	// couldn't get valid header... this should not happen unless
	// there's a runt
	if (len < sizeof(msg_header_t))
	{
	    (void) snprintf(tmpbuf,sizeof(tmpbuf)-1,"runt %ld",len);
	    log_error(peer,blockcount,"msg","parse",tmpbuf,buffer,len);
	    goto fail;
	}
	goto unknown_fail;

    }

    if ((res->head->totalLength > MAX_MESSAGE_SIZE) || (res->head->totalLength > len))
    {
	(void) snprintf(tmpbuf,sizeof(tmpbuf)-1,"totalLength %u>%ld",
		res->head->totalLength,len);
	log_error(peer,blockcount,"msg","parse",tmpbuf,(uchar8 *)0,0);
	goto fail;
    }
    if (res->head->bodyLength > res->head->totalLength)
    {
	(void) snprintf(tmpbuf,sizeof(tmpbuf)-1,"bodyLength %u>%u",
	    res->head->bodyLength, res->head->totalLength);
	log_error(peer,blockcount,"msg","parse",tmpbuf,(uchar8 *)0,0);
	goto fail;
    }
    // contentLength is greater than remaining length
    if (res->head->contentLength > len)
    {
	(void) snprintf(tmpbuf,sizeof(tmpbuf)-1,"contentLen %u>%ld",
	    res->head->contentLength, len);
	log_error(peer,blockcount,"msg","parse",tmpbuf,(uchar8 *)0,0);
	goto fail;
    }

#ifdef notdef
  if ((res->body = msg_body_from_bytes(src,len,&src,&len,
      res->head->contentLength,res->head->parityLength)) == 0)
      goto fail;

  if ((res->trail = msg_trailer_from_bytes(src,len,&src,&len)) == 0)
    goto fail;


#endif

unknown_fail:
    log_error(peer,blockcount,"msg","parse",(char *)0,(uchar8 *)0,0);
fail:
    if (res->head != 0)
	(void) free(res->head);
    if (res->body != 0)
	(void) free(res->body);
    (void) free(res);
}


// validate_message
//
// have a message that parsed but may still be invalid
// 
// unlike prior routines, we immediately can see and log the error
// 

int validate_message(msg_t *m,struct sockaddr_in *peer, long long blockcount, uchar8 *buffer, int len)
{
    char tmpbuf[128];
    // channel trashed?
    if (m->head->chanType > 1)
    {
	(void) snprintf(tmpbuf,sizeof(tmpbuf)-1,"chanType %d",m->head->chanType);
	log_error(peer,blockcount,"msg","parse",tmpbuf,(uchar8 *)0,0);
	goto fail;
    }
    // data type trashed
    if (m->head->dataType > 4)
    {
	(void) snprintf(tmpbuf,sizeof(tmpbuf)-1,"dataType %d",m->head->dataType);
	log_error(peer,blockcount,"msg","parse",tmpbuf,(uchar8 *)0,0);
	goto fail;
    }
    // total length is really busted
    if (m->head->totalLength > MAX_MESSAGE_SIZE)
    {
	(void) snprintf(tmpbuf,sizeof(tmpbuf)-1,"totalLength>max %d>%d",
		m->head->totalLength,MAX_MESSAGE_SIZE);
	log_error(peer,blockcount,"msg","parse",tmpbuf,(uchar8 *)0,0);
	goto fail;
    }
    // total length is too long
    if (m->head->totalLength > len)
    {
	(void) snprintf(tmpbuf,sizeof(tmpbuf)-1,"totalLength>len %d>%d",
		m->head->totalLength,len);
	log_error(peer,blockcount,"msg","parse",tmpbuf,(uchar8 *)0,0);
	goto fail;
    }
    // body length is busted - note this should be caught in parse
    if (m->head->bodyLength > m->head->totalLength)
    {
	(void) snprintf(tmpbuf,sizeof(tmpbuf)-1,"bodyLength %d>%d",
	    m->head->bodyLength, m->head->totalLength);
	log_error(peer,blockcount,"msg","parse",tmpbuf,(uchar8 *)0,0);
	goto fail;
    }

    // compare header and trailer -- should be equal
    if (bcmp((void *)m->head,(void *)m->trail,sizeof(m->head)) != 0)
    {
	unsigned char buffer[sizeof(*(m->head))];
	int hd;

	// right now, just generate bitwise errors
	hd = hdcmp((unsigned char *)m->head,(unsigned char *)m->trail,buffer,sizeof(*(m->head)));
	(void) snprintf(tmpbuf,sizeof(tmpbuf)-1,"HD %d",hd);
	log_error(peer,blockcount,"msg","head<>trail",tmpbuf,buffer,sizeof(*(m->head)));
    }

    // need to check parity
    // need to check crc

    return(1);

fail:
    return(0);
}
