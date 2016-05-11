
# The first rule is what is executed when you run make with no arguments
# In this case it will build two programs.
all : packetize readstream

# The executable program depends on the .o files.
# The compile line links all the .o files together to create the executable.
packetize : packetize.o crc16.o list.o
	gcc -Wall -g -o packetize packetize.o list.o crc16.o

readstream : readstream.o crc16.o list.o
	gcc -Wall -g -o readstream readstream.o list.o crc16.o


# Each individual source file depends on itself plus the header file
# Each source file can be separately compiled to produce a .o file
# that will be linked together to create the executable.
crc16.o : crc16.c data.h
	gcc -Wall -g -c crc16.c

list.o : list.c data.h
	gcc -Wall -g -c list.c

packetize.o : packetize.c data.h
	gcc -Wall -g -c packetize.c

readstream.o : readstream.c data.h
	gcc -Wall -g -c readstream.c
