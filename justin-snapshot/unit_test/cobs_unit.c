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
#include <assert.h>

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


// cobs tests
// confirm some strings encode and decode as expected
// 
// confirm various cobs routines work

#define MAGIC_BYTE 0xDE	// set bytes that should not be used to this...

static void set_to_magic(uchar8 *buf, uint64 len)
{
    uint64 i;

    for(i=0; i < len; i++)
	buf[i] = MAGIC_BYTE;
}

// test #1 -- take a sequence of special chars -- should result in COBS
// sequence of 0x1
int cobs_test1()
{
    int i;
    uchar8 buffer[512];
    uchar8 buffer2[2+(2*sizeof(buffer))];	// lots of spaces for goofs...
    uchar8 cobsbuf[2+(2*sizeof(buffer))];	// lots of space for goofs..

    bzero(buffer,sizeof(buffer));
    cobs_encode(buffer,sizeof(buffer),cobsbuf);

#ifdef NOTDEF
    {
	int j;
	for(j=0; cobsbuf[j] != 0; j++)
	    (void) fprintf(stderr,"cobs_test1(): cobsbuf[%d] = %x\n",j,cobsbuf[j]);
	(void) fprintf(stderr,"cobs_test1(): cobsbuf[%d] = %x\n",j,cobsbuf[j]);
    }
#endif

    // counterintuitive but this is right -- iterate over sizeof(buffer)
    // as each of those bytes should be a 0x1
    for(i=0; i < sizeof(buffer); i++)
    {
	if (cobsbuf[i] != 0x1)
	{
	    (void) fprintf(stderr,"unit test: cobs_encode #1 error at offset %d\n",i);
	    return(1);
	}
    }
    // now search for the zero byte
    for(i=sizeof(buffer); i < sizeof(cobsbuf); i++)
    {
	if (cobsbuf[i] == 0)
	    break;
    }
    // confirm zero byte is in right place
    if (i != sizeof(buffer))
    {
	int j;

	(void) fprintf(stderr,"unit test: cobs_encode #1 terminator at offset %d:\n",i);
	for(j=sizeof(buffer);j < i;j++)
	    (void) fprintf(stderr,"\t off %d = %x\n",j,(unsigned)cobsbuf[j]);
	return(1);
    }

    // now decode and confirm we get all zeros and don't overrun
    set_to_magic(buffer2,sizeof(buffer2));

    cobs_decode(cobsbuf,sizeof(buffer)+1,buffer2);

    // could use bcmp but that doesn't tell us which byte bad
    for(i=0; i < sizeof(buffer); i++)
    {
	if (buffer2[i] != buffer[i])
	{
	    (void) fprintf(stderr,"unit test: cobs_decode #1 off=%d,buf=%x,buf2=%x\n",
		i,(unsigned)buffer[i],(unsigned)buffer2[i]);
	    return(1);
	}

    }
    for(i=sizeof(buffer); i < sizeof(buffer2); i++)
    {
	if (buffer2[i] != MAGIC_BYTE)
	{
	    (void) fprintf(stderr,"unit test: cobs_encode #1 magic byte off %d\n",i);
	    return(1);
	}
    }
    
    return(0);
}

// test #2 -- test basic non-zero coding for 1 to 253 non-zero bytes followed by a zero
//

int cobs_test2()
{
    int i, j;
    uchar8 buffer[512];
    uchar8 buffer2[2+(2*sizeof(buffer))];	// lots of spaces for goofs...
    uchar8 cobsbuf[2+(2*sizeof(buffer))];	// lots of space for goofs..

    for(i=1; i < 254; i++)
    {
	bzero(buffer,sizeof(buffer));
	set_to_magic(buffer,i);

	// yes i+1 - want i bytes of non-zero, followed by zero
	cobs_encode(buffer,i+1,cobsbuf);

#ifdef NOTDEF
    {
	int l;

	for(l=0; l < i+1; l++)
	    (void) fprintf(stderr,"cobs_test2: buffer[%d] = %x\n",l,buffer[l]);

	for(l=0; cobsbuf[l] != 0; l++)
	    (void) fprintf(stderr,"cobs_test2: cobsbuf[%d] = %x\n",l,cobsbuf[l]);
	(void) fprintf(stderr,"cobs_test2: cobsbuf[%d] = %x\n",l,cobsbuf[l]);
    }
#endif

	if (cobsbuf[0] != (i+1))
	{
	    (void) fprintf(stderr,"unit test: cobs_encode #2 len %u <> %u\n",
		(unsigned) cobsbuf[0],(unsigned)(i+1));
	    return(1);
	}

	for(j=1; j < i+1; j++)
	{
	    if (cobsbuf[j] != MAGIC_BYTE)
	    {
		(void) fprintf(stderr,"unit test: cobs_encode #2 cobsbuf[%d] %u <> %x\n",
		    j, (unsigned) cobsbuf[j],MAGIC_BYTE);
		return(1);
	    }
	}
	if (cobsbuf[i+1] != 0x0)
	{
	    (void) fprintf(stderr,"unit test: cobs_encode #2 cobsbuf[%d] not 0x0\n",i+1);
	    return(1);
	}

	// when ready, add cobs_decode
	bzero(buffer2,sizeof(buffer2));
	cobs_decode(cobsbuf,i+1,buffer2);

	for(j=0; j < i; j++)
	{
	    if (buffer2[j] != MAGIC_BYTE)
	    {
		(void) fprintf(stderr,"unit test: cobs_encode #2 buffer2[%d] not %u\n",
		    j,MAGIC_BYTE);
		return(1);
	    }
	}
	if (buffer2[i] != 0x0)
	{
		(void) fprintf(stderr,"unit test: cobs_encode #2 buffer2[%d] not 0x0\n",j);
		return(1);
	}
    }


    return(0);
}

// test #3 -- test 254 non-zero bytes and also check 255 non-zero bytes

int cobs_test3()
{
    int i;
    uchar8 buffer[512];
    uchar8 buffer2[2+(2*sizeof(buffer))];	// lots of spaces for goofs...
    uchar8 cobsbuf[2+(2*sizeof(buffer))];	// lots of space for goofs..

    assert(sizeof(buffer) > 0xFF); // in case someone fiddles with constants

    // test 254 non-zero bytes
    bzero(buffer,sizeof(buffer));
    set_to_magic(buffer,0xFE);

    cobs_encode(buffer,0xFE,cobsbuf);

    if (cobsbuf[0] != 0xFF)
    {
	(void) fprintf(stderr,"unit test: cobs encode #3 len %u <> 0xFF\n",cobsbuf[0]);
	return(1);
    }

    for(i=1; i < 0xFF; i++)
    {
	if (cobsbuf[i] != MAGIC_BYTE)
	{
	    (void) fprintf(stderr,"unit test: cobs_encode #3 cobsbuf[%d] %u <> %x\n",
		i, (unsigned) cobsbuf[i], MAGIC_BYTE);

	    return(1);
	}
    }

    // now decode this when ready...


    // Now code 255 bytes -- so 254 plus one more non-zero byte to make sure things
    // come out right
    bzero(buffer,sizeof(buffer));
    set_to_magic(buffer,0xFF);

    cobs_encode(buffer,0xFF,cobsbuf);

    // this is right, 0xFF says 254 non-zero bytes
    if (cobsbuf[0] != 0xFF)
    {
	(void) fprintf(stderr,"unit test: cobs encode #3 len %u <> 0xFF\n",cobsbuf[0]);
	return(1);
    }

    for(i=1; i < 0xFF; i++)
    {
	if (cobsbuf[i] != MAGIC_BYTE)
	{
	    (void) fprintf(stderr,"unit test: cobs_encode #3b cobsbuf[%d] %u <> %x\n",
		i, (unsigned) cobsbuf[i], MAGIC_BYTE);

	    return(1);
	}
    }
    if (cobsbuf[0xFF] != 2)
    {
	(void) fprintf(stderr,"uni test: cobs encode #3 len %u <> 0x2\n",cobsbuf[0xff]);
	return(1);
    }

    if (cobsbuf[1+0xFF] != MAGIC_BYTE)
    {
	(void) fprintf(stderr,"unit test: cobs_encode #3c cobsbuf[%d] %u <> %x\n",
		1+0xFF, (unsigned) cobsbuf[1+0xFF], MAGIC_BYTE);
	    return(1);
    }

    return(0);
}

// test #4 -- test trashed cobs buffers and both ensure expected decode values
// and cobs_verify


// test #5 -- test partial decode
// 

int cobs_test5()
{
    int i, j, k;
    uchar8 buffer[512];
    uchar8 buffer2[2+(2*sizeof(buffer))];	// lots of spaces for goofs...

    // partial decode #1 -- fill with coded 0x0 and then ensure partial code
    // gets zeros.  This nicely ensures partial decode works when substring encoding
    // length lines up with end of partial buffer

    for(i=0; i < sizeof(buffer); i++)
	buffer[i] = 0x1;

    for(i=1; i < sizeof(buffer); i++)
    {
	set_to_magic(buffer2,sizeof(buffer2));
    
	if (cobs_partial_decode(buffer,sizeof(buffer),buffer2,i) != i)
	{
	    (void) fprintf(stderr,"unit test: cobs_partial_decode #5, extract %d\n",i);
	    return(1);
	}
#ifdef NOTDEF
	{
	    int l;
	    for(l=0; l<i; l++)
		(void) fprintf(stderr,"cobs_test5: buffer2[%d] = %x\n",l,buffer2[l]);
	}
#endif
	for(j=0; j < i; j++)
	{
	    if (buffer2[j] != 0x0)
	    {
		(void) fprintf(stderr,"unit test: cobs_partial_decode #5a, ");
		(void) fprintf(stderr,"extract %d buffer[%d] 0!=%u\n",
		    i,j,(unsigned) buffer2[0]);
		return(1);
	    }
	}

	// not clear we care about overshoot, so don't test remainder of buffer2
	// for MAGIC_BYTE
    }

    // partial decode #2 -- have some non-zero data -- both 0xFF substring and
    // regular substring

    assert(sizeof(buffer)> 0xFF);
    set_to_magic(buffer,sizeof(buffer));

    for(k=0xFE; k < (0xFF+1); k++)
    {
	buffer[0] = k;

	for(i=1; i < (k-1); i++)
	{
	    bzero(buffer2,sizeof(buffer2));
	    if (cobs_partial_decode(buffer,sizeof(buffer),buffer2,i) != i)
	    {
		(void) fprintf(stderr,"unit test: cobs_partial_decode #5b, ");
		(void) fprintf(stderr,"extract %d/%d\n",k,i);
		return(1);
	    }
	    for(j=0; j < i; j++)
	    {
		if (buffer2[j] != MAGIC_BYTE)
		{
		    (void) fprintf(stderr,"unit test: cobs_partial_decode #5c, ");
		    (void) fprintf(stderr,"extract %d/%d buffer[%d] %d!=%u\n",
			k,i,j,(unsigned) buffer2[0],MAGIC_BYTE);
		    return(1);
		}
	    }
	}
    }

    // partial decode #3 -- cause partial decode to overshoot by 1 byte and > 1 bytes
    assert(sizeof(buffer)> 0xFF);
    set_to_magic(buffer,sizeof(buffer));

    buffer[0] = 0x5;	// so 4 bytes of content followed by zero

    // 4 bytes of buffer too small for 5 bytes
    if (cobs_partial_decode(buffer,4,buffer2,5) != 0)
    {
	(void) fprintf(stderr,"unit test: cobs_partial_code #5d, extract 5\n");
	return(1);
    }
    buffer[5] = 0;
    // 5 bytes of valid content too small for 6 bytes
    if (cobs_partial_decode(buffer,6,buffer2,6) != 0)
    {
	(void) fprintf(stderr,"unit test: cobs_partial_code #5e, extract 6\n");
	return(1);
    }

    return(0);
}


// top level routine

int cobs_tests()
{
    if (cobs_test1() != 0)
	return(1);

    if (cobs_test2() != 0)
	return(1);

    if (cobs_test3() != 0)
	return(1);

    if (cobs_test5() != 0)
	return(1);

    return(0);

}
