# by tirra <tirra.newly@gmail.com> (c) 2006, 2007, 2008
# makefile for redleaf modulas sources

CFLAGS=-g -Wall -shared -nostartfiles

INCLUDES+=../src/include/

all: defaults

defaults: cgi_modula

cgi_modula: cgi_modula.c
	@echo "MD: cgi_modula.c"
	@gcc ${CFLAGS} -I${INCLUDES} -o cgi_modula.so cgi_modula.c

clean:
	@echo "RM: *.so modulas"
	@rm -f *.so

