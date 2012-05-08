CC=gcc
CFLAGS=-Wall -O3 -Werror -std=c99
LDFLAGS=-lm -pthread -lft -lz -ljson

flowy: flowy.o ftreader.o auto_comps.o utils.o grouper_fptr.o merger_fptr.o auto_assign.o iterate.o error_functions.o base.c
	$(CC) $^ $(LDFLAGS) -o $@

clean:
	rm -f flowy *.o auto_assign.c auto_comps.c auto_comps.h auto_switch.c
