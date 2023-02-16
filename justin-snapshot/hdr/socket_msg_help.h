#ifndef SOCKET_MSG_HELP_H
#define SOCKET_MSG_HELP_H

#include "./message.h"
#include "./stdtypes.h"

//Purpose: send a message struct on a socket
//Parameters: a socket and a message pointer
//Returns: none
//Note: none
//Warnings: NOT THREAD SAFE YET
extern void send_msg(int sock, msg_t* src);

//Purpose: receive one message struct on a socket
//Parameters: a socket to read from
//Returns: a message pointer formed from reading the socket
//Notes: none
//Warnings: NOT THREAD SAFE YET
extern msg_t* recv_msg(int sock);

extern int msg_to_cobs(msg_t* src, uchar8 **buf, uint64 *len);

#endif//SOCKET_MSG_HELP_H
