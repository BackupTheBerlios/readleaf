# by tirra <tirra.newly@gmail.com> (c) 2007
# RedLeaf makefile

-include config
 
ETCDIR = ${DESTDIR}/etc
SHAREDIR = ${DESTDIR}/share/redleaf

all: defaults

defaults: redleafd

config.h: config.h.in
	@echo "CFG: Configuring ..."
	@./genconfh.sh

redleafd: config.h 
	@echo "BLD: Compiling redleafd ..."
	@cd src; make

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
	@cd src; make clean
	@rm -f config.h

