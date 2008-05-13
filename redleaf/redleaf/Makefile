# by tirra <tirra.newly@gmail.com> (c) 2007
# RedLeaf makefile

PREFIX=/usr/local
ETCDIR=/usr/local/etc
SHAREDIR=${PREFIX}/share/redleaf

defaults: src/redleafd

src/redleafd:
	@echo "BLD: Compiling redleafd ..."
	@cd src; make

install: src/redleafd
	@echo "INS: creating directories ..."
	@mkdir -p ${PREFIX}/bin
	@mkdir -p ${SHAREDIR}/www
	@mkdir -p ${ETCDIR}/redleaf
	@echo "INS: stripping binary ..."
	@strip redleafd
	@echo "INS: copying files ..."
	@cp redleafd ${PREFIX}/bin/redleafd
	@cp example.conf ${ETCDIR}/redleaf/example.conf
	@echo "INS: done"

clean: 
	@echo "MKF: cleaning ..."
	@cd src; make clean

