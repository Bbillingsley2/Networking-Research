#CFLAGS=-Wall -g -DDEBUG2
CFLAGS=-Wall  

OBJ= main.o cobs.o message.o socket_msg_help.o \
parity.o byte_help.o stdtypes.o byte_order.o socket_help.o handle_errors.o \
read_blocks.o error_util.o


qdclient: $(OBJ)
	gcc -o qdclient $(OBJ)
	cp qdclient ../../test/qdclient

main.o: main.c ../server/hipft.h ../justin-snapshot/hdr/cobs.h handle_errors.h

read_blocks.o: read_blocks.c handle_errors.h ../server/hipft.h

handle_errors.o: handle_errors.c handle_errors.h

error_util.o: error_util.c error_util.h

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


clean:
	- rm -f $(OBJ) qdclient



