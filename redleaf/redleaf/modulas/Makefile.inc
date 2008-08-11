# by tirra <tirra.newly@gmail.com> (c) 2006, 2007, 2008
# makefile for redleaf modulas sources

#CFLAGS+=-shared -nostartfiles

MODS=modulas/cgi_modula.so

modulas/cgi_modula.so: 
	@echo "[MOD] cgi_modula.so"
	@gcc ${CFLAGS} -shared -nostartfiles -fPIC -I$(INCLUDES) modulas/cgi_modula.c -o modulas/cgi_modula.so

modulas_inst:
	@echo "[INS] modulas ..."
	@cp modulas/*.so ${SHAREDIR}/

modulas_rm:
	@echo "[RM] *.so modulas"
	@rm -f modulas/*.so

