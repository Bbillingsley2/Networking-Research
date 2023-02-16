#ifndef SERVER_HELP_H
#define SERVER_HELP_H

#include "./stdtypes.h"
#include "./conn_chan.h"

//Purpose: find an existing channel if there is one in the connection double pointer list
//Parameters: a connection double pointer list, the conenction index where the channel is,
//            and details about the channel you are finding
//Returns: a pointer to a channel pointer and a pointer to its index
//         (given back via the parameters)
//Note: uses sendID and recvID to find the channel
extern void find_existing_channel(connection_t*** conns, uint64 conn_index, uint64 sendID, uint64 recvID, channel_t** rchan, uint64* rindex);

//Purpose: fork a channel (a thread) to handle client-related tasks
//Parameters: a forced arbitrary pointer that is a channel_t pointer
//Returns: returns NULL always
//Note: the server will interact with the channel via the channel_t flags
extern void* fork_server_channel(void* ptr);

#endif//SERVER_HELP_H