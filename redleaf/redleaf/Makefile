# by tirra <tirra.newly@gmail.com> (c) 2007
# RedLeaf makefile

DESTDIR=/usr/local
ETCDIR=/usr/local/etc
SHAREDIR=${DESTDIR}/share/redleaf

all: defaults

defaults: redleafd

redleafd: 
	@echo "BLD: Compiling redleafd ..."
	@cd src; make

install: src/redleafd
	@echo "INS: creating directories ..."
	@mkdir -p ${DESTDIR}/bin
	@mkdir -p ${SHAREDIR}/www
	@mkdir -p ${ETCDIR}/redleaf
	@echo "INS: stripping binary ..."
	@strip src/redleafd
	@echo "INS: copying files ..."
	@cp src/redleafd ${DESTDIR}/bin/redleafd
	@cp example.conf ${ETCDIR}/redleaf/example.conf
	@cp www-default/index.html ${SHAREDIR}/www/index.html
	@cp 
	@echo "INS: done"

clean: 
	@echo "MKF: cleaning ..."
	@cd src; make clean

