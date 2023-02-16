#include <netdb.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include <ctype.h>

#include "../hdr/message.h"
#include "../hdr/stdtypes.h"
#include "../hdr/byte_help.h"
#include "../hdr/socket_msg_help.h"
#include "../hdr/section.h"

#define MAX_SIZE 8000

uint64 establish_channel(int sock, uint64 selfID, uint64* otherID, uint32 maxSize, uchar8* file, uint32 fileLength){
    uint64 temp;
    msg_t* send = form_msg(selfID, *otherID, CONTROL, SYN, maxSize, file, fileLength);
    send_msg(sock, send);
    free_msg(send);
    msg_t* recv = recv_msg(sock);
    while(verify_msg(recv) != true){
        free_msg(recv);
        send = form_msg(selfID, *otherID, CONTROL, RTS, maxSize, file, fileLength);
        send_msg(sock, send);
        free_msg(send);
        recv = recv_msg(sock);
    }
    *otherID = recv->head->sendID;

    temp = send->head->segmentNum; // total segments define
    if(recv->head->sendID == 0){
        syslog(LOG_ALERT, "hipERROR: Channel could not be Created.");
        temp = 0;
    }else{
        syslog(LOG_ALERT, "Channel is Created for a file %s with %lu packets.", file, recv->head->segmentNum);
    }
    free_msg(recv);
    return temp;
}
void ready_channel(int sock, uint64 selfID, uint64 otherID, uint32 maxSize, uchar8* file, uint32 fileLength){
    msg_t* send = form_msg(selfID, otherID, CONTROL, SYN, maxSize, file, fileLength);
    send_msg(sock, send);
    free_msg(send);
    syslog(LOG_ALERT, "Channel is Ready and Active.");
}

bool8 gather_file(int sock, uint64 selfID, uint64 otherID, uint64 maxSize, uint64 numfilepcks, char8* outfile){
    // move to channel establish
    if(numfilepcks == 0){
        syslog(LOG_ALERT, "Desired File could not be found server-side.");
        return false;
    }

    uint64 numpcks = 0;
    msg_t* pck = NULL;
    FILE* fhandle = fopen(outfile,"wb"); // check for value
    section_t* retrans_sec = create_section(); // check

    while(numpcks <= numfilepcks){
        pck = recv_msg(sock);
        if(pck == NULL){
            syslog(LOG_ALERT, "hipERROR: Can not Read NULL PACKET.");
            return false;
        }
        if(verify_msg(pck) == false){
            add_to_section(retrans_sec, numpcks+1);
        }else{
            // use fseek (create a gap)
            fwrite(pck->body->content,sizeof(uchar8),pck->head->contentLength,fhandle);
        }
        free_msg(pck);
        numpcks++;
    }

    msg_t* send;
    while(get_piece_in_section(retrans_sec, 0) != NULL){
        send = form_msg(selfID, otherID, CONTROL, RTS, first_low_in_section(retrans_sec), NULL, 0);
        send_msg(sock, send);
        free_msg(send);
        pck = recv_msg(sock);
        if(pck == NULL){
            syslog(LOG_ALERT, "hipERROR: Can not Read NULL PACKET.");
            return false;
        }
        if(verify_msg(pck) != false){
            // any retrans not just the first
            if(first_low_in_section(retrans_sec) < numfilepcks){
                fseek(fhandle, (int32)( (first_low_in_section(retrans_sec)-1) * maxSize ), SEEK_SET);
                fwrite(pck->body->content,sizeof(uchar8),pck->head->contentLength,fhandle);
            }
            delete_from_section(retrans_sec, first_low_in_section(retrans_sec));
        }
        free_msg(pck);
    }
    fclose(fhandle);
    syslog(LOG_ALERT, "Desired File was found and written.");
    return true;
}
void cease_channel(int sock, uint64 selfID, uint64 otherID){
    msg_t* send = form_msg(selfID, otherID, CONTROL, FIN, 0, NULL, 0);
    send_msg(sock, send);
    free_msg(send);
    msg_t* recv = recv_msg(sock);
    verify_msg(recv);
    // check verify
    free_msg(recv);
    syslog(LOG_ALERT, "Channel closed.");
}
void cease_connection(int sock){
    msg_t* send = form_msg(0, 0, CONTROL, FIN, 0, NULL, 0);
    send_msg(sock, send);
    free_msg(send);
    syslog(LOG_ALERT, "Connection to the Server is Closed.");
    close(sock);
}

int main(int argc, char8** argv){
  //TODO: use getopt
    openlog("hipFT-client", LOG_PERROR|LOG_PID, LOG_USER);
    char8* output_filename = "./files/output/output.txt";
	int port;
	int sock = -1;
	struct sockaddr_in address;
	struct hostent* host;

	//checking commandline parameter
	if(argc != 5){
		printf("usage: %s hostname port filename #downloads\n", argv[0]);
		return -1;
	}

    //checking #downloads parameter
    int numDownloads = atoi((char8*)argv[4]);
    if(numDownloads<0){
        printf("#downloads (%s) must be a postive integer (0 or above)\n", argv[3]);
        return -4;
    }

	//obtain port number
	if(sscanf(argv[2], "%d", &port) <= 0){
		fprintf(stderr, "%s: error: wrong parameter: port\n", argv[0]);
		return -2;
	}

	//create socket
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock <= 0){
		fprintf(stderr, "%s: error: cannot create socket\n", argv[0]);
		return -3;
	}

	//connect to server
	address.sin_family = AF_INET;
	address.sin_port = htons((uint16)port);
	host = gethostbyname(argv[1]);
	if(!host){
		fprintf(stderr, "%s: error: unknown host %s\n", argv[0], argv[1]);
		return -4;
	}


	memcpy(&address.sin_addr, host->h_addr_list[0], (size_t)host->h_length);
	if (connect(sock, (struct sockaddr *)&address, sizeof(address))){
		fprintf(stderr, "%s: error: cannot connect to host %s\n", argv[0], argv[1]);
		return -5;
	}

    syslog(LOG_ALERT, "Connection to a Server is Established.");

    uint64 selfID = 0;
    uint64 otherID = 0;
    uint64 temp;
    int cycle = 0;
    bool8 runForever = (numDownloads == 0) ? true : false;

    while(cycle < numDownloads || runForever){
        selfID = generate_random_uint(100, 5000000000);
        otherID = 0;
        temp = establish_channel(sock, selfID, &otherID, MAX_SIZE, (uchar8*)argv[3], (uint32)strlen(argv[3]));
        ready_channel(sock, selfID, otherID, MAX_SIZE, (uchar8*)argv[3], (uint32)strlen(argv[3]));
        gather_file(sock, selfID, otherID, MAX_SIZE, temp, output_filename);
        cease_channel(sock, selfID, otherID);
        cycle = cycle + 1;
    }

    //close and clean
    cease_connection(sock);
    closelog();
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
	return 0;
}
