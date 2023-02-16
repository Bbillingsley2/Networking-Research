CFLAGS=-Wall -DDEBUG2
#CFLAGS=-Wall
OBJ= main.o server.o file_transfer.o log.o cobs.o message.o socket_msg_help.o \
		parity.o byte_help.o stdtypes.o byte_order.o socket_help.o
qdserver: $(OBJ)
	gcc -o qdserver $(OBJ)
	cp qdserver ../../test/

clean:
	- rm -f $(OBJ) qdserver

main.o: main.c hipft.h server.h

server.o: server.c hipft.h server.h file_transfer.h

file_transfer.o: file_transfer.c hipft.h file_transfer.h log.h

log.o: log.c log.h

cobs.o: ../justin-snapshot/src/cobs.c
	gcc -c $(CFLAGS) -I../justin-snapshot/hdr/ ../justin-snapshot/src/cobs.c

message.o: ../justin-snapshot/src/message.c
	gcc -c $(CFLAGS) -I../justin-snapshot/hdr/ ../justin-snapshot/src/message.c

socket_msg_help.o: ../justin-snapshot/src/socket_msg_help.c
	gcc -c $(CFLAGS) -I../justin-snapshot/hdr/ ../justin-snapshot/src/socket_msg_help.c

parity.o: ../justin-snapshot/src/parity.c
	gcc -c $(CFLAGS) -I../justin-snapshot/hdr/ ../justin-snapshot/src/parity.c

byte_help.o: ../justin-snapshot/src/byte_help.c
	gcc -c $(CFLAGS) -I../justin-snapshot/hdr/ ../justin-snapshot/src/byte_help.c

byte_order.o: ../justin-snapshot/src/byte_order.c
	gcc -c $(CFLAGS) -I../justin-snapshot/hdr/ ../justin-snapshot/src/byte_order.c

stdtypes.o: ../justin-snapshot/src/stdtypes.c
	gcc -c $(CFLAGS) -I../justin-snapshot/hdr/ ../justin-snapshot/src/stdtypes.c

socket_help.o: ../justin-snapshot/src/socket_help.c
	gcc -c $(CFLAGS) -I../justin-snapshot/hdr/ ../justin-snapshot/src/socket_help.c

