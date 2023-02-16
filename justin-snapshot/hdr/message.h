#ifndef MESSAGE_H
#define MESSAGE_H

#include "./stdtypes.h"
#include "../hdr/parity.h"
#include "../hdr/byte_help.h"

//Purpose: given some info about what the channel will do/act
typedef enum{    // CHANNEL_TYPE
  DATA = 0,      // normal data operations
  CONTROL = 1    // control the channel in some way
} CHANNEL_TYPE;
//Purpose: basic flags for communication
typedef enum{    // DATA_TYPE
  SYN = 0,       // normal new data
  ACK = 1,       // acknowledge
  RTS = 2,       // retransmission data
  NACK = 3,      // nack data
  FIN = 4        // finish communications
} DATA_TYPE;
//Purpose: turn the CHANNEL_TYPE enum to a string value thats printable
extern const char8* const NAME_CHANNEL_TYPE[];
//Purpose: turn the DATA_TYPE enum to a string value thats printable
extern const char8* const NAME_DATA_TYPE[];

//Purpose: the message protcol definition for a header
typedef struct{
  uint16 mbz;			// must be zero
  uint8 chanType;           // Data, Control
  uint8 dataType;           // SYN, ACK, RTS, FIN
  uint32 msgID;	            // random message ID, matched in trailer
  uint32 totalLength;	      // total length of message
  uint32 bodyLength;	      // total length of body
  uint32 contentLength;     // total length of content [not incl. padding]
  uint32 parityLength;      // total length of vparity + hparity [not incl. padding]
  uint64 sendID;            // sender ID
  uint64 recvID;            // receiver ID
  uint64 segmentNum;        // segment number that the packet is corresponding to (file transfer)
  uint64 crc;               // message crc
} msg_header_t;

//Purpose: the message protcol definition for a body
typedef struct{
  uchar8* content;          // data [not incl. padding to 32 bytes chunks]
  uint64* vparityData;      // vertical parity data, size: 4 * uint64
  uchar8* hparityData;      // horizontal parity data, size: depends on data
} msg_body_t;

//Purpose: the message protcol definition for a trailer
typedef struct{
  uint16 mbz;			// must be zero
  uint8 chanType;           // Data, Control
  uint8 dataType;           // SYN, ACK, RTS, FIN
  uint32 msgID;	            // random message ID, matched in trailer
  uint32 totalLength;	      // total length of message
  uint32 bodyLength;	      // total length of body
  uint32 contentLength;     // total length of content [not incl. padding]
  uint32 parityLength;      // total length of vparity + hparity [not incl. padding]
  uint64 sendID;            // sender ID
  uint64 recvID;            // receiver ID
  uint64 segmentNum;        // segment number that the packet is corresponding to (file transfer)
  uint64 crc;               // message crc
} msg_trailer_t;

//Purpose: the message protcol definition for a complete message
typedef struct{
  msg_header_t* head;       // the message's header
  msg_body_t* body;         // the message's body
  msg_trailer_t* trail;     // the message's trailer
} msg_t;

extern uint64 calculate_header_length(void);
extern uint64 calculate_trailer_length(void);
extern uint64 calculate_parity_length(uint64 bufflen);
extern uint64 calculate_body_length(uint64 clen);
extern uint64 calculate_total_length(uint64 clen);

extern void free_msg(msg_t* src);
extern msg_t* form_msg(uint64 sendID, uint64 recvID, uint8 chanType, uint8 dataType, uint64 segmentNum, uchar8* content, uint32 length);
extern void msg_to_bytes(msg_t* src, uchar8** dst, uint64* len);
extern msg_t* msg_from_bytes(uchar8* src, uint64 len);
//extern msg_t* msg_from_bytes(uchar8* src, uint64);
extern void print_msg(msg_t* src, bool8 printContent, bool8 printvParity, bool8 printhParity);
extern bool8 verify_msg(msg_t* src);
extern msg_header_t *msg_header_from_bytes(uchar8 *src, uint64 len, uchar8 **consumed, uint64* remain);
extern msg_body_t *msg_body_from_bytes(uchar8 *src, uint64 len, uchar8 **consumed, uint64* remain, uint64 contentlen, uint64 paritylen);
extern msg_trailer_t *msg_trailer_from_bytes(uchar8 *src, uint64 len, uchar8 **consumed, uint64* remain);

#define MAX_MESSAGE_SIZE 65536

#endif//MESSAGE_H
