CC=gcc
CFLAGS= -Wall -g
LDLIBS= -lm

ALL=tsh

all : $(ALL)
tsh : tsh.c

clean:
	rm -rf *~ $(ALL)