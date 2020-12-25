CC=gcc
CFLAGS= -Wall -g
LDLIBS= -lm

tsh : tsh.o tar.o cd.o tsh_memory.o string_traitement.o exec_funcs.o pwd.o mkdir.o ls.o rmdir.o rm.o simpleCommande.o redirection.o bloc.o cp.o


tsh.o : tsh.c cd.h tsh_memory.h pwd.h mkdir.h tar.h ls.h rmdir.h simpleCommande.h
tar.o : tar.c tar.h
cd.o : cd.c tar.h tsh_memory.h string_traitement.h
tsh_memory.o : tsh_memory.c tsh_memory.h string_traitement.h
string_traitement.o : string_traitement.c string_traitement.h tsh_memory.h
exec_funcs.o : exec_funcs.c tsh_memory.h cd.h
pwd.o : pwd.c pwd.h
mkdir.o : mkdir.c tar.h tsh_memory.h string_traitement.h cd.h
ls.o : ls.c tar.h tsh_memory.h exec_funcs.h
rmdir.o: rmdir.c tar.h tsh_memory.h string_traitement.h exec_funcs.h
rm.o: rm.c cd.h tsh_memory.h exec_funcs.h
redirection.o : redirection.c tar.h simpleCommande.h cd.h tsh_memory.h string_traitement.h mkdir.h rm.h
simpleCommande.o : simpleCommande.c tsh_memory.h cd.h pwd.h mkdir.h ls.h rmdir.h string_traitement.h cp.h

cleanall:
	rm -rf *.o main *~
clean:
	rm -rf *~
