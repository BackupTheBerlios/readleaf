# by tirra <tirra.newly@gmail.com> (c) 2007, 2008
# RedLeaf makefile

-include Makefile.rules
-include src/Makefile.inc
-include src/libdata/Makefile.inc
-include src/liballoc/Makefile.inc
ifeq ($(MODULAS),yes)
-include modulas/Makefile.inc
endif

REDLEAFDOBJ=$(LIBDATA_OBJS) $(LIBALLOC_OBJS) $(DAEMON_OBJS)

BUILDR=redleafd
CFLAGS=-g -Wall

ifeq ($(MODULAS),yes)
#BUILDR+=modules_all
LLIBS+=-rdynamic -ldl
endif

ETCDIR = ${DESTDIR}/etc
SHAREDIR = ${DESTDIR}/share/redleaf

%.o: %.c
	@echo "[CC] $^"
	@gcc $(CFLAGS) -Isrc/include/ -c -o $@ $^

all: defaults

defaults: $(BUILDR)

redleafd: $(REDLEAFDOBJ) config.h
	@echo "BLD: Compiling redleafd ..."
	@gcc $(REDLEAFDOBJ) -o src/redleafd $(LLIBS)

install: redleafd
	@echo "INS: creating directories ..."
	@mkdir -p ${DESTDIR}/bin
	@mkdir -p ${SHAREDIR}/www
	@mkdir -p ${SHAREDIR}/www/img
	@mkdir -p ${ETCDIR}/redleaf
	@echo "INS: stripping binary ..."
	@strip src/redleafd
	@echo "INS: copying files ..."
	@cp src/redleafd ${DESTDIR}/bin/redleafd
	@cp example.conf ${ETCDIR}/redleaf/example.conf
	@cp www-default/index.html ${SHAREDIR}/www/index.html
	@cp www-default/form.html ${SHAREDIR}/www/form.html
	@cp www-default/img/redleaflogo.png ${SHAREDIR}/www/img/redleaflogo.png
	@echo "INS: done"

clean: 
	@echo "MKF: cleaning ..."
	@rm -f src/redleafd
	@rm -f $(REDLEAFDOBJ)


