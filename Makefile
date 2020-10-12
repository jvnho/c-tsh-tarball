CC=gcc
CFLAGS= -Wall -g
LDLIBS= -lm


tsh : tsh.o cd.o tsh_memory.o string_traitement.o

tsh.o : tsh.c cd.h tsh_memory.h
cd.o : cd.c tar.h tsh_memory.h string_traitement.h
tsh_memory.o : tsh_memory.c tsh_memory.h string_traitement.h
string_traitement.o : string_traitement.c string_traitement.h

cleanall:
	rm -rf *.o main *~ 
clean:
	rm -rf *~