#ifndef SOCKET_HELP_H
#define SOCKET_HELP_H

#include <unistd.h>

//Purpose: read in all the bits even if they come in late
//Parameters: a socket, an allocated buffer to place the bytes in, and
//            the number of bytes that are suppose to be read in
//Returns: the number of bytes acutally read in from the socket
//Notes: text-book, thread-safe
extern ssize_t readn(int fd, void* buffer, ssize_t n);

//Purpose: write in all the bits even if not all the bytes can be wrote
//         at one time
//Parameters: a socket, a buffer filled with bytes to write, and
//            the number of bytes that are suppose to be wrote in
//Returns: the number of bytes acutally written in the socket
//Notes: text-book, thread-safe
extern ssize_t writen(int fd, const void* buffer, ssize_t n);

#endif//SOCKET_HELP_H