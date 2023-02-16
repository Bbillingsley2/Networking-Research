#ifndef CONN_CHAN_H
#define CONN_CHAN_H

#include <sys/socket.h>
#include "./stdtypes.h"
#include "./message.h"
#include "./file_handle.h"

typedef struct{
  //specs
  int sock;
  pthread_t thread;
  uint64 selfID;
  uint64 otherID;
  uint64 maxSize;
  file_handle_t* handle;
  //flags
  uint8 data;
  uint8 kill;
  uint8 ready;
  //shared_data
  msg_t* value;
} channel_t;

typedef struct{
  int sock;
  struct sockaddr address;
  socklen_t addrLen;
  uint64 numChans;
  channel_t** chans;
} connection_t;

//connection functions
extern connection_t* new_connection(connection_t*** conns, uint64* totalConnections);
extern void print_connection(connection_t*** conns, uint64* totalConnections, uint64 index);
extern void end_connection(connection_t*** conns, uint64* totalConnections, uint64* totalChannels, uint64 conn_index);
extern void print_all_connections(connection_t*** conns, uint64* totalConnections);
extern void end_all_connections(connection_t*** conns, uint64* totalConnections, uint64* totalChannels);

//channel functions
extern channel_t* new_channel(connection_t*** conns, uint64* totalChannels, uint64 conn_index);
extern void end_channel(connection_t*** conns, uint64* totalChannels, uint64 conn_index, uint64 chan_index);

#endif//CONN_CHAN_H