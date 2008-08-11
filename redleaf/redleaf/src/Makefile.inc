# redleafd daemon sources makefile
# Originally written by Tirra <tirra.newly@gmail.com>
# (c) ReadLeaf team 2008

DAEMON_OBJS=src/main.o src/serv.o src/misc.o src/http.o \
	src/conf.o src/file.o src/page.o src/http_read_dir.o 

ifeq ($(MODULAS),yes)
	DAEMON_OBJS += src/modula.o
endif

