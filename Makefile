CC=gcc
CFLAGS= -Wall -g
LDLIBS= -lm

tsh : tsh.o tar.o cd.o tsh_memory.o string_traitement.o pwd.o mkdir.o ls.o rmdir.o rm.o simpleCommande.o


tsh.o : tsh.c cd.h tsh_memory.h pwd.h mkdir.h tar.h ls.h rmdir.h simpleCommande.h
tar.o : tar.c tar.h
cd.o : cd.c tar.h tsh_memory.h string_traitement.h
tsh_memory.o : tsh_memory.c tsh_memory.h string_traitement.h
string_traitement.o : string_traitement.c string_traitement.h
pwd.o : pwd.c pwd.h
mkdir.o : mkdir.c tar.h tsh_memory.h string_traitement.h cd.h
ls.o : ls.c tar.h tsh_memory.h
rmdir.o: rmdir.c tar.h tsh_memory.h string_traitement.h
rm.o: rm.c cd.h tsh_memory.h
simpleCommande.o : simpleCommande.c tsh_memory.h cd.h pwd.h mkdir.h ls.h rmdir.h tsh_memory.h string_traitement.h

cleanall:
	rm -rf *.o main *~
clean:
	rm -rf *~
