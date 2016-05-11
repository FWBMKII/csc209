
# Note that the tabs at the beginning of each of the gcc lines
# is necessary.

tester : tester.o rectangle.o
	gcc -Wall -g -o tester tester.o rectangle.o

tester.o : tester.c rectangle.h
	gcc -Wall -g -c tester.c

rectangle.o : rectangle.c rectangle.h
	gcc -Wall -g -c rectangle.c
