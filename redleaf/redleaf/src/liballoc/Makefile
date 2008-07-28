#
# Makefile for liballoc
# tirra <tirra.newly@gmail.com> (c) 2006, 2007, 2008
#

INCLUDES+=../include/
CFLAGS=-Wall -g
OBJS= bbuddy.o memmap.o


default: all

.c.o:
	@echo "CC: $*.o"
	@gcc $(CFLAGS) -I${INCLUDES} -c -o $*.o $< 

liballoc.a: $(OBJS)
	@ar rcs liballoc.a $(OBJS)

all: liballoc.a
	@echo "AR: liballoc.a"

clean: 
	@echo ">> Cleaning liballoc objects ..."
	@rm -r *.o
