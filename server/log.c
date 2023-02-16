#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <time.h>

/*
 * produce a CSV log entry recording what the server did
 *
 * yyyy.mm.dd.hh.min.sec,src IP,src port,dst IP,dst port,#blocks/packets,#bytes,any errors
 */

void hipftlog(const struct sockaddr_in *src,const struct sockaddr_in *dst,
	const long long blockcount,const long long bytecount,const int errors)
{
    time_t now_t;
    struct tm *now_tm;

    now_t = time((time_t *)0);
    now_tm = gmtime(&now_t);

    (void) printf("%d.%02d.%02d.%02d.%02d.%02d,",1900+now_tm->tm_year,1+now_tm->tm_mon,now_tm->tm_mday,
    	now_tm->tm_hour,now_tm->tm_min,now_tm->tm_sec);
    (void) printf("%s,%d,%s,%d,",inet_ntoa(src->sin_addr),ntohs(src->sin_port),
	inet_ntoa(dst->sin_addr),ntohs(dst->sin_port));
    (void) printf("%lld,%lld,%d\n",blockcount,bytecount,errors);
    (void) fflush(stdout);	/* probably not needed */
}


