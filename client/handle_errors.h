extern void handle_null_packet(struct sockaddr_in *,long long);
extern void handle_cobs_errors(struct sockaddr_in *,long long ,uchar8 *,int);
extern void handle_busted_msg1(struct sockaddr_in *,long long ,uchar8 *,uint64);
extern int validate_message(msg_t *,struct sockaddr_in *,long long,uchar8 *, int);
