# by tirra <tirra.newly@gmail.com> (c) 2007, 2008
# RedLeaf makefile

-include Makefile.rules
-include src/Makefile.inc
-include src/libdata/Makefile.inc
-include src/liballoc/Makefile.inc

all: defaults

ifeq ($(MODULAS),yes)
-include modulas/Makefile.inc
endif

REDLEAFDOBJ = $(LIBDATA_OBJS) $(LIBALLOC_OBJS) $(DAEMON_OBJS)

BUILDR = redleafd
CFLAGS = -g -Wall

ifeq ($(MODULAS),yes)
BUILDR += $(MODS)
LLIBS += -rdynamic -ldl
EXTRA_CLEAN += modulas_rm
EXTRA_INSTALL += modulas_inst
endif

ETCDIR = ${DESTDIR}/etc
SHAREDIR = ${DESTDIR}/share/redleaf
INCLUDES = src/include/

%.o: %.c
	@echo "[CC] $^"
	@gcc $(CFLAGS) -I$(INCLUDES) -c -o $@ $^

defaults: $(BUILDR)

redleafd: $(REDLEAFDOBJ) config.h
	@echo "[BLD] Linking redleafd ..."
	@gcc $(REDLEAFDOBJ) -o redleafd $(LLIBS)

pre_inst:
	@echo "[INS] creating directories ..."
	@mkdir -p ${DESTDIR}/bin
	@mkdir -p ${SHAREDIR}/www
	@mkdir -p ${SHAREDIR}/www/img
	@mkdir -p ${ETCDIR}/redleaf

install: $(BUILDR) pre_inst $(EXTRA_INSTALL)
	@echo "[INS] stripping binary ..."
	@strip redleafd
	@echo "[INS] copying files ..."
	@cp redleafd ${DESTDIR}/bin/redleafd
	@cp example.conf ${ETCDIR}/redleaf/example.conf
	@cp www-default/index.html ${SHAREDIR}/www/index.html
	@cp www-default/form.html ${SHAREDIR}/www/form.html
	@cp www-default/img/redleaflogo.png ${SHAREDIR}/www/img/redleaflogo.png
	@echo "[INS] done"

clean: $(EXTRA_CLEAN)
	@echo "[MKF] cleaning daemon stuff ..."
	@echo "[RM] redleafd ..."
	@rm -f redleafd
	@echo "[RM] daemon objects ..."
	@rm -f $(REDLEAFDOBJ)



