#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <time.h>

#include "hipft.h"
#include "server.h"
#include "file_transfer.h"

/*#define DEBUG 0*/

/*
 * Signal handler to handle SIGCHLD in parent
 */

static void reaper(int signal_number)
{
    int wstatus;

    if (signal_number != SIGCHLD)
	return;

    (void) wait (&wstatus);	/* just toss status */
}


/*
 * start the server
 */

int start_server(const char *progname, struct sockaddr_in *oursin)
{
    int s;
    struct sigaction sa;

    if ((s = socket(PF_INET,SOCK_STREAM,0)) < 0)
	goto error;

    /* set up port structure ... */
    oursin->sin_family = AF_INET;
    oursin->sin_port = htons(HIPFT_PORT);
    oursin->sin_addr.s_addr = INADDR_ANY;

    if (bind(s,(struct sockaddr *)oursin,sizeof(*oursin)) < 0)
	goto error;

    /* NOTE: setting backlog to arbitrary value */
    if (listen(s,10) < 0)
	goto error;

    /*
     * now, because we are going to have children, arrange to cleanup children
     */
    bzero((void *)&sa,sizeof(sa));
    sa.sa_handler = &reaper;
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD,&sa,(struct sigaction *)0) != 0)
	goto error;

    return(s);

error:
    (void) close(s);
    perror(progname);
    return(-1);
}

/*
 * take incoming connection, create a child, call child file xfer routine
 * and on parent side, log start and cleanup
 */

void run_server(const int s, const char *progname, struct sockaddr_in *oursin, const int blocksize)
{
    int f;
    int s2;
    struct sockaddr_in sin;
    socklen_t sinlen;

    sinlen = sizeof(sin);
    while ((s2 = accept(s,(struct sockaddr *)&sin, &sinlen)) > 0)
    {
	sinlen = sizeof(sin);	/* nuisance of accept */

	f = fork();

	if (f == 0)
	{
	    /*
	     * in child -- tidy up and then run file transfer
	     * note not doing an exec() as child is simple enough it doesn't
	     * impact the state still shared with parent and exec() has its own
	     * hazards
	     */
	    (void) close(s); 
	    run_file_transfer(s2,oursin,&sin,blocksize);
	    exit(0);
	}

	/* in parent */
	if (f < 0)
	{
	    /* should only happen if we mess up reaping children */
	    perror(progname);
	    return;
	}

	(void) close(s2);
    }
    if (s2 < 0)
	(void) fprintf(stdout,"error from accept\n");
}
