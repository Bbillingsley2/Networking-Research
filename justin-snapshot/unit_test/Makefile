CFLAGS=-Wall -g -DDEBUG
#CFLAGS=-Wall -g

OBJ = main.o cobs.o message.o socket_msg_help.o \
    parity.o byte_help.o stdtypes.o byte_order.o socket_help.o cobs_unit.o


unit: $(OBJ)
	gcc -o unit $(OBJ)

main.o: main.c

cobs_unit.o: cobs_unit.c 

cobs.o: ../src/cobs.c
	gcc -c $(CFLAGS) -I../hdr/ ../src/cobs.c

message.o: ../src/message.c
	gcc -c $(CFLAGS) -I../hdr/ ../src/message.c

socket_msg_help.o: ../src/socket_msg_help.c
	gcc -c $(CFLAGS) -I../hdr/ ../src/socket_msg_help.c

parity.o: ../src/parity.c
	gcc -c $(CFLAGS) -I../hdr/ ../src/parity.c

byte_help.o: ../src/byte_help.c
	gcc -c $(CFLAGS) -I../hdr/ ../src/byte_help.c

byte_order.o: ../src/byte_order.c
	gcc -c $(CFLAGS) -I../hdr/ ../src/byte_order.c

stdtypes.o: ../src/stdtypes.c
	gcc -c $(CFLAGS) -I../hdr/ ../src/stdtypes.c

socket_help.o: ../src/socket_help.c
	gcc -c $(CFLAGS) -I../hdr/ ../src/socket_help.c


clean:
	- rm -f $(OBJ) unit
