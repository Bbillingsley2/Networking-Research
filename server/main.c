
#include <sys/types.h>
#include <sys/socket.h>

#include <netdb.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "hipft.h"
#include "server.h"

/*
 * init(): read arguments
 */

int init(int argc, char **argv, char **progname, int *blocksize)
{
    /*
     * currently no arguments to read
     * eventually will set server directory, etc
     */

    *progname = *argv;
    *blocksize = HIPFT_BLOCK_SIZE;
    return(0);
}

/*
 * secure(): make server safe to run
 * pre-load any shared binaries
 * change to directory with files
 * does chroot
 * setuid
 * redirect STDOUT to our logfile
 * close STDIN
 * leave STDERR available for now for debugging and the like
 */

int secure(const char *progname)
{
    time_t now_t;
    struct tm *now_tm;

    /* force loading of shared network libraries */
    (void) gethostbyname("localhost");

    if ((now_t = time((time_t *)0)) == -1)
    {
	perror(progname);
	return(1);
    }

    if ((now_tm = localtime(&now_t)) == 0)
    {
	perror(progname);	/* not good enough message... */
	return(1);
    }


    /* chdir ... */
    if (chdir(HIPFT_PATH) != 0)
    {
	perror(HIPFT_PATH);	/* not good enough */
	return(1);
    }

    /* eventually need to do chroot and setuid here... */

    /* file initialization */

    (void) fclose(stdin);	/* don't need stdin */

    {
	char logname[1024]; /* warning, constant... */

	/*
	 * produces log named server.yyyy.mm.dd.hh.min.sec
	 *
	 * note using 1 logfile across forked instances...
	 * NOT safe if we have heavy logging!
	 */
	(void) snprintf(logname,sizeof(logname),
	    "log/s.%.4d.%0.2d%0.2d.%0.2d%0.2d%0.2d.csv",
	    now_tm->tm_year+1900,now_tm->tm_mon+1,now_tm->tm_mday,
	    now_tm->tm_hour,now_tm->tm_min,now_tm->tm_sec);
	if (freopen(logname,"w",stdout) == NULL)
	{
	    perror(logname);
	    return(1);
	}
    }
    return(0);
}

/*
 * main itself
 */

int main(int argc, char **argv)
{
    char *progname;
    int s;
    int blocksize;
    struct sockaddr_in local_sin;	/* who we are */

    if (init(argc,argv,&progname,&blocksize) != 0)
	return(1);

    if (secure(progname) != 0)
	return(1);

    if ((s = start_server(progname,&local_sin)) < 0)
	return(1);

    run_server(s,progname,&local_sin,blocksize);

    /* should never get here, but... */
    return(0);
}

