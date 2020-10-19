CC=gcc
CFLAGS= -Wall -g
LDLIBS= -lm


tsh : tsh.o cd.o tsh_memory.o string_traitement.o pwd.o mkdir.o

tsh.o : tsh.c cd.h tsh_memory.h pwd.h mkdir.h
cd.o : cd.c tar.h tsh_memory.h string_traitement.h
tsh_memory.o : tsh_memory.c tsh_memory.h string_traitement.h
string_traitement.o : string_traitement.c string_traitement.h
pwd.o : pwd.c pwd.h
mkdir.o : mkdir.c 


cleanall:
	rm -rf *.o main *~ 
clean:
	rm -rf *~