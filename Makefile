CC=gcc
CFLAGS= -Wall -g
LDLIBS= -lm


tsh : tsh.o cd.o

tsh.o : tsh.c cd.h
cd.o : cd.c tar.h

cleanall:
	rm -rf *.o main *~ 
clean:
	rm -rf *~