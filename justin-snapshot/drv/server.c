#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <pthread.h>
#include <linux/in.h>
#include <syslog.h>

#include "../hdr/conn_chan.h"
#include "../hdr/socket_msg_help.h"
#include "../hdr/message.h"
#include "../hdr/server_help.h"
#include "../hdr/file_handle.h"

#define NUM_THREADS (4)
#define MAX_CHANNELS (NUM_THREADS-1)
char8* fileDirectory = "./files/input";

int main(int argc, char8** argv){
    //persistent vars
    openlog("hipFT-server", LOG_PERROR|LOG_PID, LOG_USER);
    connection_t** conns = NULL;
    uint64 totalConnections = 0;
    uint64 totalChannels = 0;
    dir_handle_t* handles = open_dir_handle(strlen(fileDirectory), (uchar8*)fileDirectory);
    print_dir_handle(handles);
    printf("\n");

    //reusable vars
    int port;
    int master_socket, new_socket, addrlen, activity, max_sd;
    int opt = (int)true;
    fd_set readfds; //set of socket descriptors
    struct sockaddr_in address;

    //checking commandline parameter
    if(argc != 2){
		printf("usage: %s port\n", argv[0]);
		return -1;
	}

    //obtain port number
	if(sscanf(argv[1], "%d", &port) <= 0){
		fprintf(stderr, "%s: error: wrong parameter: port\n", argv[1]);
		return -2;
	}

    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0 ){
        syslog(LOG_ALERT, "hipERROR: Failed to Start the Socket.");
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char8 *)&opt, sizeof(opt)) < 0 ){
        syslog(LOG_ALERT, "hipERROR: Allowing Multiple Connections on the Socket Failed.");
        exit(EXIT_FAILURE);
    }

    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( (uint16)port );

    //bind the socket to localhost port
    if(bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0){
        syslog(LOG_ALERT, "hipERROR: Socket Bind Failed.");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d\n", port);

    //max 10 connections on master_socket
    if(listen(master_socket, 5) < 0){
        syslog(LOG_ALERT, "hipERROR: Socket Listening Failed.");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    printf("Waiting for connections...\n");

    //start the main loop
    connection_t* connect;
    channel_t* chan;
    msg_t* sendmsg;
    msg_t* recvmsg;
    struct sockaddr_in* caddr;
    uint64 tempInt;
    file_handle_t* fhandle;
    while(true){
        //reset
        connect = NULL;
        chan = NULL;
        sendmsg = NULL;
        recvmsg = NULL;
        caddr = NULL;
        tempInt = 0;
        fhandle = NULL;

        //clear the socket set
        FD_ZERO(&readfds);
        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;
        //add child sockets to set
        for(uint64 i=0; i<totalConnections; i++){
            if(conns[i]->sock > 0){ //if valid socket descriptor then add to read list
                FD_SET(conns[i]->sock, &readfds);
            }
            if(conns[i]->sock > max_sd){ //highest file descriptor number, need it for the select function
                max_sd = conns[i]->sock;
            }
        }

        //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
        activity = select(max_sd + 1 , &readfds , NULL , NULL , NULL);
        // clean specific error descriptor, find which one messed up
        if((activity < 0) && (errno!=EINTR)){
            syslog(LOG_ALERT, "hipERROR: Select Error Occured.");
        }

        //if something happened on the master socket, then its an incoming connection
        if (FD_ISSET(master_socket, &readfds)){
            //accept incoming connections
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0){
                syslog(LOG_ALERT, "hipERROR: Accepting Incoming Connection Error Occured.");
                exit(EXIT_FAILURE);
            }
            //inform user of socket number and channel number - used in send and receive commands
            printf("New connection, socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
            // store the connection
            connect = new_connection(&conns, &totalConnections);
            connect->sock = new_socket;
            connect->addrLen = (socklen_t)addrlen;
            connect->address = *(struct sockaddr*)&address;
            connect->numChans = 0;
            connect->chans = NULL;
        }

        //else its some IO operation on some other socket
        for(uint64 i=0; i<totalConnections; i++){
            caddr = (struct sockaddr_in*)&(conns[i]->address);
            if(FD_ISSET(conns[i]->sock, &readfds)){
                //check if it was for closing and read the incoming message
                recvmsg = recv_msg(conns[i]->sock);
                if(recvmsg == NULL){
                    //disconnect
                    //recover for that channel
                    end_connection(&conns, &totalConnections, &totalChannels, i);
                    printf("Automatic Host disconnect, ip %s , port %d \n", inet_ntoa(caddr->sin_addr), ntohs(caddr->sin_port));
                }else{
                    if(verify_msg(recvmsg)==false){
                        //send a RTS message
                        sendmsg = form_msg(recvmsg->head->recvID, recvmsg->head->sendID, CONTROL, RTS, 0, NULL, 0);
                        send_msg(conns[i]->sock, sendmsg);
                        free_msg(sendmsg);
                        free_msg(recvmsg);
                        continue;
                    }
                    // connection control
                    if(recvmsg->head->sendID==0 && recvmsg->head->recvID==0){
                        if(recvmsg->head->chanType==CONTROL && recvmsg->head->dataType==FIN){
                            //disconnect
                            end_connection(&conns, &totalConnections, &totalChannels, i);
                            printf("Manual Host disconnect, ip %s , port %d \n", inet_ntoa(caddr->sin_addr), ntohs(caddr->sin_port));
                            free_msg(recvmsg);
                        }else{
                            //nack message "idk"
                            sendmsg = form_msg(recvmsg->head->recvID, recvmsg->head->sendID, CONTROL, NACK, 0, NULL, 0);
                            send_msg(conns[i]->sock, sendmsg);
                            free_msg(sendmsg);
                            free_msg(recvmsg);
                        }
                    }else if(recvmsg->head->sendID>0 && recvmsg->head->recvID==0){
                        if(recvmsg->head->chanType==CONTROL && (recvmsg->head->dataType==SYN || recvmsg->head->dataType==RTS)){
                            //channel creation/info
                            fhandle = file_handle_for_file(handles, (uchar8*)recvmsg->body->content, recvmsg->head->contentLength);
                            if((totalChannels >= MAX_CHANNELS && fhandle == NULL) || fhandle == NULL){
                                sendmsg = form_msg(0, recvmsg->head->sendID, CONTROL, FIN, 0, NULL, 0);
                                send_msg(conns[i]->sock, sendmsg);
                            }else if(totalChannels >= MAX_CHANNELS){
                                sendmsg = form_msg(0, recvmsg->head->sendID, CONTROL, FIN, 0, recvmsg->body->content, recvmsg->head->contentLength);
                                send_msg(conns[i]->sock, sendmsg);
                            }else{
                                find_existing_channel(&conns, i, recvmsg->head->sendID, 0, &chan, &tempInt);
                                if(chan == NULL){
                                    chan = new_channel(&conns, &totalChannels, i);
                                    chan->sock = conns[i]->sock;
                                    chan->otherID = recvmsg->head->sendID;
                                    chan->selfID = generate_random_uint(300, 46744073709551615);
                                    chan->maxSize = recvmsg->head->segmentNum;
                                    chan->handle = fhandle;
                                    pthread_create(&chan->thread, 0, fork_server_channel, (void *) chan);
                                }
                                sendmsg = form_msg(chan->selfID, recvmsg->head->sendID, CONTROL, ACK, num_segments_in_file_handle(chan->handle, chan->maxSize), NULL, 0);
                                send_msg(conns[i]->sock, sendmsg);
                            }
                            free_msg(sendmsg);
                            free_msg(recvmsg);
                        }else if (recvmsg->head->chanType==CONTROL && recvmsg->head->dataType==FIN){
                            //end channel if it exists
                            find_existing_channel(&conns, i, recvmsg->head->sendID, recvmsg->head->recvID, &chan, &tempInt);
                            if(chan != NULL){ end_channel(&conns, &totalChannels, i, tempInt); }
                            sendmsg = form_msg(0, recvmsg->head->sendID, CONTROL, FIN, 0, NULL, 0);
                            send_msg(conns[i]->sock, sendmsg);
                            free_msg(sendmsg);
                            free_msg(recvmsg);
                        }else{
                            //nack message "idk"
                            sendmsg = form_msg(recvmsg->head->recvID, recvmsg->head->sendID, CONTROL, NACK, 0, NULL, 0);
                            send_msg(conns[i]->sock, sendmsg);
                            free_msg(sendmsg);
                            free_msg(recvmsg);
                        }
                    }else{
                        if(recvmsg->head->chanType==CONTROL && recvmsg->head->dataType==SYN){
                            //ready a channel
                            find_existing_channel(&conns, i, recvmsg->head->sendID, recvmsg->head->recvID, &chan, &tempInt);
                            if(chan != NULL){
                                chan->ready = true;
                            }else{
                                //nack message "idk"
                                sendmsg = form_msg(recvmsg->head->recvID, recvmsg->head->sendID, CONTROL, NACK, 0, NULL, 0);
                                send_msg(conns[i]->sock, sendmsg);
                                free_msg(sendmsg);
                            }
                            free_msg(recvmsg);
                        }else if(recvmsg->head->chanType==CONTROL && recvmsg->head->dataType==RTS){
                            //retransmission
                            find_existing_channel(&conns, i, recvmsg->head->sendID, recvmsg->head->recvID, &chan, &tempInt);
                            if(chan != NULL){
                                chan->value = recvmsg;
                                chan->data = true;
                            }else{
                                //nack message "idk"
                                sendmsg = form_msg(recvmsg->head->recvID, recvmsg->head->sendID, CONTROL, NACK, 0, NULL, 0);
                                send_msg(conns[i]->sock, sendmsg);
                                free_msg(sendmsg);
                                free_msg(recvmsg);
                            }
                        }else if(recvmsg->head->chanType==CONTROL && recvmsg->head->dataType==FIN){
                            //end channel if it exists
                            find_existing_channel(&conns, i, recvmsg->head->sendID, recvmsg->head->recvID, &chan, &tempInt);
                            if(chan != NULL){ end_channel(&conns, &totalChannels, i, tempInt); }
                            sendmsg = form_msg(0, recvmsg->head->sendID, CONTROL, FIN, 0, NULL, 0);
                            send_msg(conns[i]->sock, sendmsg);
                            free_msg(sendmsg);
                            free_msg(recvmsg);
                        }else{
                            //nack message "idk"
                            sendmsg = form_msg(recvmsg->head->recvID, recvmsg->head->sendID, CONTROL, NACK, 0, NULL, 0);
                            send_msg(conns[i]->sock, sendmsg);
                            free_msg(sendmsg);
                            free_msg(recvmsg);
                        }
                    }
                }
            }
        }
    }
    end_all_connections(&conns, &totalConnections, &totalChannels);
    close_dir_handle(handles);
    closelog();
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
    return 0;
}
