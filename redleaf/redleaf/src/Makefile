# by tirra <tirra.newly@gmail.com> (c) 2006, 2007
# makefile for redleaf sources

SOURCES=http.c http_read_dir.c conf.c misc.c serv.c main.c page.c file.c
HEADERS=include/http.h include/conf.h include/file.h include/misc.h \
	include/serv.h include/file.h include/http_read_dir.h
CFLAGS=-g -Wall
OBJS=http.o conf.o misc.o serv.o main.o http_read_dir.o page.o file.o libdata/libdata.a
INCLUDES+=include/

all: defaults

defaults: redleafd $(HEADERS)

.c.o: $(SOURCES)
	@echo "CC: $*.o"
	@gcc $(CFLAGS) -I${INCLUDES} -c -o $*.o $<

libdata/libdata.a:
	@echo "BLD: Building libdata.a ..."
	@cd libdata;make

redleafd: $(OBJS) 
	@gcc $(OBJS) -o redleafd

clean:
	@echo "RM: *.o"
	@rm -f $(OBJS)
	@cd libdata;make clean
	@echo "RM: redleafd"
	@rm -f redleafd



