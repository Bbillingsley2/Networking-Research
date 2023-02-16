#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>

#include "file_transfer.h"
#include "log.h"

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


/*
 * actually ship the file
 */

static long shipfile(int s,const char *filename,
	const int blocksize, long long *blockcount, long long *bytecount)
{
    FILE *fp;
    char *databuf;
    int len;
    msg_t *m;


    /* handle various obvious files we don't even try to open */
    if ((filename == 0) || (filename[0] == '\0') || (filename[0] == '.'))
        return(0);

    if ((fp = fopen(filename,"r")) == NULL)
    {
	(void) perror(filename);
        return(1);
    }

    databuf = (char *)malloc(blocksize);
    len = 0;
    *bytecount = *blockcount = 0;

    /*
     * got the file, now read each block
     * for now, read only full blocks
     */
    while ((len = fread(databuf,1,(size_t)blocksize,fp)) == blocksize)
    {
	m = form_msg(0,0,0,0,0,(uchar8 *)databuf,len);
	print_msg(m,1,0,0);
	send_msg(s,m);
	free_msg(m);

	(*bytecount) += len;
        (*blockcount)++;
    }

    (void) free(databuf);
    (void) fclose(fp);
    return(0);
}

/*
 * do the file transfer
 * arguments are socket, sockaddr of remote peer, and blocksize
 */

void run_file_transfer(int s, struct sockaddr_in *src, struct sockaddr_in *dst,
	const int blocksize)
{
    DIR *dp;
    struct dirent *dep;
    long long blocktotal, blockcount;
    long long bytetotal, bytecount;
    int errors;

    errors = 0;

    if ((dp = opendir(".")) == NULL)
    {
	perror("opendir");
        return;
    }

    bytetotal = blocktotal = 0;

    /* errno before and after so we can check errno as we leave loop */
    for(errno=0,dep=readdir(dp); dep!= NULL; errno=0,dep=readdir(dp))
    {
        /* other types are ignored - e.g. avoid symlinks due to hazards */

        if (dep->d_type & DT_REG)
        {
	    if (shipfile(s,dep->d_name,blocksize,&blockcount,&bytecount) != 0)
		errors++;
            blocktotal += blockcount;
	    bytetotal += bytecount;
        }

    }

    if (errno != 0)
	errors++;

    closedir(dp);
    hipftlog(src,dst,blocktotal,bytetotal,errors);
}
