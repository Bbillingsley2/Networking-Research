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

extern long long read_blocks(int s,struct sockaddr_in *peer);

#define RECV_BUFFER_SIZE 64000000

/*
 * super simple client to start testing server
 */

int main(int argc, char **argv)
{
    int s;
    long long blockcount;
    /* Recv buffer size */
    int recvbuf=RECV_BUFFER_SIZE;
    size_t len;
    socklen_t socklen = sizeof(recvbuf);
    socklen_t i = sizeof(recvbuf);
    struct sockaddr_in sin;

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
	perror("socket");
	return(1);
    }

    if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, &recvbuf, socklen) < 0) {
        perror("setsockopt");
	return(1);
    }
    (void) fprintf(stderr, "receive buffer length = %d\n",recvbuf);

#ifdef DEBUG
    (void) fprintf(stderr, "Supplied recv buffer length = %ul\n",recvbuf);
    i = sizeof(len);
    if (getsockopt(s, SOL_SOCKET, SO_RCVBUF, &len, &i) < 0) {
        perror(": getsockopt");
    }
    (void) fprintf(stderr, "System recv buffer size = %lu\n", len);
#endif


    bzero((void *)&sin,sizeof(sin));

    sin.sin_family = AF_INET;
    sin.sin_port = htons(HIPFT_PORT);
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(s,(struct sockaddr *)&sin,sizeof(sin)) != 0)
    {
	perror("connect");
	return(1);
    }

    blockcount = read_blocks(s,&sin);

    (void) fprintf(stderr, "received %lld blocks\n",blockcount);
    return(0);
}
