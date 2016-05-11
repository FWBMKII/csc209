CC=gcc
PORT=50526
CFLAGS = -DPORT=$(PORT) -g -Wall

xmodemserver : xmodemserver.o crc16.o helper.o client1.o
	${CC} ${CFLAGS} -o xmodemserver xmodemserver.o  crc16.o helper.o


crc16.o : crc16.c crc16.h
	${CC} ${CFLAGS} -c crc16.c

helper.o : helper.c 
	${CC} ${CFLAGS} -c helper.c

xmodemserver.o : xmodemserver.c xmodemserver.h crc16.h
	${CC} ${CFLAGS} -c xmodemserver.c 
