CC=gcc
CFLAGS= -Wall -g
LDLIBS= -lm


tsh : tsh.o cd.o tsh_memory.o

tsh.o : tsh.c cd.h tsh_memory.h
cd.o : cd.c tar.h tsh_memory.h
tsh_memory.o : tsh_memory.c tsh_memory.h

cleanall:
	rm -rf *.o main *~ 
clean:
	rm -rf *~