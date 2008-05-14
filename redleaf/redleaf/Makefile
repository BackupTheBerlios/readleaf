# by tirra <tirra.newly@gmail.com> (c) 2007
# RedLeaf makefile

PREFIX=/usr/local
ETCDIR=/usr/local/etc
SHAREDIR=${PREFIX}/share/redleaf

all: defaults

defaults: redleafd

redleafd: 
	@echo "BLD: Compiling redleafd ..."
	@cd src; make

install: src/redleafd
	@echo "INS: creating directories ..."
	@mkdir -p ${PREFIX}/bin
	@mkdir -p ${SHAREDIR}/www
	@mkdir -p ${ETCDIR}/redleaf
	@echo "INS: stripping binary ..."
	@strip src/redleafd
	@echo "INS: copying files ..."
	@cp src/redleafd ${PREFIX}/bin/redleafd
	@cp example.conf ${ETCDIR}/redleaf/example.conf
	@echo "INS: done"

clean: 
	@echo "MKF: cleaning ..."
	@cd src; make clean

