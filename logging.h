#ifndef LOGGING_H
#define LOGGING_H

extern char* get_current_time(void);
extern void log_network_errors(uint32_t, const struct sockaddr_in*, long long, char*, char*, char*, uchar8*, int)
extern void log_trace_msg(uint32_t, char const* const, ...)
extern void log_server_errors(uint32_t, const struct sockaddr_in*, const struct sockaddr_in*, const long long, const long long, const int)


#endif