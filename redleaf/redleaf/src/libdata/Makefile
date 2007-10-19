#
# Makefile for the libdata.
# tirra <tirra.newly@gmail.com> (c) 2006,2007
#

INCLUDES+=../include/
CFLAGS=-Wall -g
OBJS= usrtc.o tree.o lslist.o avl.o splay.o redblack.o


default: all

.c.o:
	@echo "CC: $*.o"
	@gcc $(CFLAGS) -I${INCLUDES} -c -o $*.o $< 

libdata.a: $(OBJS)
	@ar rcs libdata.a $(OBJS)

all: libdata.a
	@echo "AR: libdata.a"

clean: 
	@echo ">> Cleaning libdata objects ..."
	@rm -r *.o

