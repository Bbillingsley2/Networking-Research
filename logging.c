#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <strings.h>

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

//#include "handle_errors.h"
//#include "error_util.h"



/*
 * These headers are just for testing. Clean up before merging
 */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BASIC_DEBUG             1
#define NETWORK_DEBUG           2
#define SERVER_DEBUG            4
#define PARSE_MSG_DEBUG         8

unsigned char flags;

flags = BASIC_DEBUG|NETWORK_DEBUG|SERVER_DEBUG|PARSE_MSG_DEBUG;

/*
 * Get current time for logging
 */

char * get_current_time(void)
{

    time_t cur_time;
    struct tm * timeinfo;

    time(&cur_time);
    timeinfo = localtime(&cur_time); //convert to local time

    static char _retval[24] = {0};
    strftime(_retval, sizeof(_retval), "%Y-%m-%d %H:%M:%S", timeinfo); //format as a string
    return _retval;
}

static void log_network_errors(uint32_t LEVEL_TRACED, const struct sockaddr_in *peer, long long bcount,
	char *type, char *subtype, char *specific, uchar8 *pkt, int pktlen)
{
    if ((flags & NETWORK_DEBUG) == NETWORK_DEBUG) {
    (void) fprintf(stderr, "%s [LOG LEVEL: %d] [%s() %s:%d] ", get_current_time(), LEVEL_TRACED, __func__, __FILE__, __LINE__);
    (void) fprintf(stdout, "%s,%lld,",inet_ntoa(peer->sin_addr),bcount);
    (void) fprintf(stdout, "%s,",type);

    // subtype and specific
    //
    (void) fprintf(stdout,"%s,%s,",(subtype==0?"unknown":subtype),
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
    (void) fprintf(stdout,",");

    (void) fprintf(stdout,"\n");
    }
}


void log_trace_msg(uint32_t LEVEL_TRACED, char const * const fmt, ...)
{
      if ((flags & BASIC_DEBUG) == BASIC_DEBUG) {
	    va_list argp;
	    va_start(argp, fmt);
	    (void) fprintf(stderr, "%s [LOG LEVEL: %d] [%s() %s:%d] ", get_current_time(), LEVEL_TRACED, __func__, __FILE__, __LINE__);
	    (void) vfprintf(stderr, fmt, argp);
	    (void) fprintf(stdout,",");
	    (void) fprintf(stdout,"\n");
	    va_end(argp);
	}
}



void log_server_errors(uint32_t LEVEL_TRACED, const struct sockaddr_in *src,const struct sockaddr_in *dst,
	const long long blockcount,const long long bytecount,const int errors)
{
    if ((flags & SERVER_DEBUG) == SERVER_DEBUG)
    {
        (void) fprintf(stdout, "%s [LOG LEVEL: %d] [%s() %s:%d] ", get_current_time(), LEVEL_TRACED, __func__, __FILE__, __LINE__);
        (void) fprintf(stdout, "%s:%d,%s:%d,",inet_ntoa(src->sin_addr),ntohs(src->sin_port),
        inet_ntoa(dst->sin_addr),ntohs(dst->sin_port));
        (void) printf("%lld,%lld,%d\n",blockcount,bytecount,errors);
        (void) fflush(stdout);	/* probably not needed */
    }
}


void log_parse_msg_error(uint32_t LEVEL_TRACED, const struct sockaddr_in *peer, long long bcount, char* type,
    char* subtype, char* specific, uchar8* pkt, int pktlen)
{
    if ((flags & PARSE_MSG_DEBUG) == PARSE_MSG_DEBUG)
    {
        (void) fprintf(stderr, "%s [LOG LEVEL: %d] [%s() %s:%d] ", get_current_time(), LEVEL_TRACED, __func__, __FILE__, __LINE__);
        (void) fprintf(stdout, "%s,%lld,",inet_ntoa(peer->sin_addr),bcount);
    
        // subtype and specific
        //
        (void) fprintf(stdout,"%s,%s,",(subtype==0?"unknown":subtype),
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

        (void) fprintf(stdout,",");
	    (void) fprintf(stdout,"\n");
    }
}


//This can go away before merge
int main(int argc, char ** argv)
{
    /*
     * test case 1
     */
    log_trace_msg(BASIC_DEBUG, "This is a freeformed debug message");


    struct sockaddr_in sin;
    bzero((void * ) & sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(9999);
    sin.sin_addr.s_addr = inet_addr("129.82.45.22");
    long blockcount = 0;
    long bytecount = 0;
    char *tmpbuf = {0};

    /*
     * test case 2
     */
    log_network_errors(NETWORK_DEBUG, &sin,blockcount,"cobs","null packet",(char *)0,(uchar8 *)0,0);
    /*
     * test case 3
     */
    log_server_errors(SERVER_DEBUG, &sin, &sin, blockcount, bytecount,0);
}
