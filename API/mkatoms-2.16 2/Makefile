srcdir = .
top_srcdir = .

prefix = /var/www/html/ASP/API
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/share/info
mandir = ${prefix}/share/man
includedir = ${prefix}/include

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL}

CC = gcc
CPP = @CPP@
CXX = @CXX@
CXXCPP = @CXXCPP@
RANLIB = @RANLIB@

DEFS = -DHAVE_CONFIG_H -I. -I$(srcdir) -I..
CPPFLAGS = 
LDFLAGS = 
LDLIBS = 
CXXFLAGS = @CXXFLAGS@ $(DEFS)
CFLAGS = -g -O2 $(DEFS)
RM=rm -f

LDIST=dist

DEFAULT_SHELL=/bin/bash
BAT_EXT=

GETONE=getone$(BAT_EXT)
GETALL=getall$(BAT_EXT)

CFLAGS += -O3 -Wall

.PHONY: all static install clean distclean

all: mkatoms

-include Makefile-local

mkatoms: mkatoms.c

static: CFLAGS += -static -static-libgcc
static: clean mkatoms
	@echo "Static version compiled as mkatoms"

install: mkatoms
	@if [ ! -z "$(BAT_EXT)" ]; then \
		cp getone $(GETONE) ;\
		cp getall $(GETALL) ;\
	fi
	@-( umask 22 ; mkdir -p $(bindir) ; chmod 0755 $(bindir) )
	@$(INSTALL_PROGRAM) mkatoms $(bindir)
	@$(INSTALL_PROGRAM) $(GETONE) $(bindir)
	@$(INSTALL_PROGRAM) $(GETALL) $(bindir)

clean:
	rm -f mkatoms *.o

distclean: clean
	rm -fR config.cache config.h config.log config.status *config.cache autom4te.cache getall getone Makefile
