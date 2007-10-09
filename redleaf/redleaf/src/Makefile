
# makefile for redleaf sources

SOURCES=http.c conf.c misc.c serv.c main.c
CFLAGS=-Wall
OBJS=http.o conf.o misc.o serv.o main.o
INCLUDES+=include/

all: defaults

defaults: redleafd

.c.o: $(SOURCES)
	@echo "CC: $*.o"
	@gcc $(CFLAGS) -I${INCLUDES} -c -o $*.o $<

redleafd: $(OBJS)
	@gcc $(OBJS) -o redleafd

clean:
	@echo "RM: *.o"
	@rm -f $(OBJS)
	@echo "RM: redleafd"
	@rm -f redleafd
