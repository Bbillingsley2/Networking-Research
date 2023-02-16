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

#define BLOCK_INITIAL_SIZE 8192
#define ERROR 0


#ifdef ERROR
int randchange(int c) {
        c ^= random();
	return(c);
}
#endif

/*
 * returns number of blocks read
 */

long long read_blocks(int s,struct sockaddr_in *peer)
{
    int c;
    uchar8 *buffer, *decode_buffer;
    int blen, bindex;
    long long blockcount, chunkcount;
    msg_t *m;
    FILE *sfp;
#ifdef ERROR
    int errorcounter;
#endif

    /*
     * convert s to a FILE * for buffered reading
     */
    sfp = fdopen(s,"r");
    chunkcount = blockcount = 0;
#ifdef ERROR
    errorcounter = 0;
#endif

    do
    {
	/* get a buffer */
	if ((buffer = (uchar8 *)malloc(BLOCK_INITIAL_SIZE)) == NULL)
	{
	    perror("malloc");
	    goto bail;

	}

	blen = BLOCK_INITIAL_SIZE;
	bzero(buffer,BLOCK_INITIAL_SIZE);

	decode_buffer = 0;
	bindex = 0;
	m = (msg_t *)0;
	
	while ((c = fgetc(sfp)) != EOF)
	{
//#ifdef ERROR
//	    // insert random errors every ERRFREQ input chars
//	    errorcounter++;
//	    if (errorcounter == ERRFREQ)
//	    {
//		c = randchange(c);
//		errorcounter = 0;
//	    }
//#endif 
	    if (c == 0X00) /* COBS delimiter? */
	    {
		/* handle terminated COBS buffer of len bindex */

		chunkcount++;

		if (bindex == 0)
		{
		    handle_null_packet(peer,chunkcount);
		    break;
		}


#ifdef DEBUG2
		(void) fprintf(stderr,"Received message of %d bytes\n",bindex+1);
#endif

		if (alt_cobs_verify(buffer, bindex) != true)
		{
#ifdef DEBUG2
		    (void) fprintf(stderr,"alt_cobs_verify\n");
#endif
		    handle_cobs_errors(peer,chunkcount,buffer,bindex);
		    break;
		}

		/* 
		 * this works because bindex, by def of COBS, is > than length of
		 * decoded data
		 */

		decode_buffer = malloc(bindex);
		cobs_decode(buffer, bindex, decode_buffer);

		/* update bindex to new length */
		bindex = cobs_decoded_length_from_encoded(buffer);
#ifdef DEBUG2
		print_hex(decode_buffer, bindex);
#endif
		if ((m = msg_from_bytes(decode_buffer,bindex)) == 0)
		{
		    handle_busted_msg1(peer,chunkcount,buffer,bindex);
		    break;
		}
		
		if (validate_message(m,peer,chunkcount,buffer,bindex))
		{
		    /* increment valid blockcount */
		    blockcount++;
		}
		break; /* break from inner while loop */
	    }
	    /* not COBS delimiter, add to buffer */
	    buffer[bindex++] = (char) c;
	    if (bindex == blen)
	    {
		blen += BLOCK_INITIAL_SIZE;
		if ((buffer = (uchar8 *)realloc((void *)buffer,blen))==NULL)
		{
		    perror("realloc");
		    goto bail;
		}
	    }
	} // end inner while

	// free buffers for next packet
	(void) free(buffer);

	if (decode_buffer != 0)
	    (void) free(decode_buffer);
    }
    while (c != EOF);

bail:
    /* don't close sfp - apparently close/shutdown issues arise */
    return(blockcount);
}
