CC=gcc
STRIP=strip
XDEFINES=
XLIBS=
XLIBPATHS=
XIPATHS=
PREFIX=/usr/local
XHYDRA_SUPPORT=xhydra
STRIP=strip

#
# Makefile for Hydra - (c) 2001-2006 by van Hauser / THC <vh@thc.org>
#

OPTS=-I. -Wall -O2
# -ggdb -pedantic
LIBS=-lm
DIR=/bin

SRC = hydra-vnc.c hydra-pcnfs.c hydra-rexec.c hydra-nntp.c hydra-socks5.c \
      hydra-telnet.c hydra-cisco.c hydra-http.c hydra-ftp.c hydra-imap.c \
      hydra-pop3.c hydra-smb.c hydra-icq.c hydra-cisco-enable.c hydra-ldap.c \
      hydra-mysql.c hydra-http-proxy.c hydra-mssql.c hydra-smbnt.c \
      hydra-snmp.c hydra-cvs.c hydra-smtpauth.c hydra-sapr3.c hydra-ssh2.c \
      hydra-teamspeak.c hydra-postgres.c hydra-rsh.c hydra-rlogin.c \
      hydra-oracle-listener.c hydra-svn.c hydra-pcanywhere.c hydra-sip.c hydra-vmauthd.c \
      hydra-http-proxy-auth-ntlm.c hydra-imap-ntlm.c hydra-pop3-ntlm.c hydra-smtpauth-ntlm.c \
      hydra-http-form.c crc32.c d3des.c md4.c ntlm.c hydra-mod.c hydra.c
OBJ = hydra-vnc.o hydra-pcnfs.o hydra-rexec.o hydra-nntp.o hydra-socks5.o \
      hydra-telnet.o hydra-cisco.o hydra-http.o hydra-ftp.o hydra-imap.o \
      hydra-pop3.o hydra-smb.o hydra-icq.o hydra-cisco-enable.o hydra-ldap.o \
      hydra-mysql.o hydra-http-proxy.o hydra-smbnt.o hydra-mssql.o \
      hydra-snmp.o hydra-cvs.o hydra-smtpauth.o hydra-sapr3.o hydra-ssh2.o\
      hydra-teamspeak.o hydra-postgres.o hydra-rsh.o hydra-rlogin.o \
      hydra-oracle-listener.o hydra-svn.o hydra-pcanywhere.o hydra-sip.o hydra-vmauthd.o \
      hydra-http-proxy-auth-ntlm.o hydra-imap-ntlm.o hydra-pop3-ntlm.o hydra-smtpauth-ntlm.o \
      hydra-http-form.o crc32.o d3des.o md4.o hydra-mod.o ntlm.o hydra.o
BIN = hydra
BINUNIX = hydra pw-inspector
BINWIN = hydra.exe pw-inspector.exe

EXTRA_DIST = README README.arm README.palm CHANGES TODO INSTALL LICENSE.GNU LICENCE.HYDRA \
             hydra-mod.h hydra.h crc32.h d3des.h md4.h

all:	pw-inspector hydra $(XHYDRA_SUPPORT) 
	@echo
	@echo Now type "make install"

hydra:	$(OBJ)
	$(CC) $(OPTS) $(LIBS) -o $(BIN) $(OBJ) $(LIBS) $(XLIBS) $(XLIBPATHS) || echo -e "\nIF YOU RECEIVED THE ERROR MESSAGE \"cannot find -lpq\" DO THE FOLLOWING:\n  make clean; ./configure\n  vi Makefile    <- and remove the \"-lpq\" and \"-DLIBPOSTGRES\" statements\n  make\n"
	@echo
	@echo If men could get pregnant, abortion would be a sacrament
	@echo

xhydra:	
	-cd hydra-gtk && ./make_xhydra.sh

pw-inspector: pw-inspector.c
	-$(CC) $(OPTS) -o pw-inspector pw-inspector.c

.c.o:	
	$(CC) $(OPTS) -c $< $(XDEFINES) $(XIPATHS)

strip:	hydra
	-test -e hydra.exe && touch Makefile || strip $(BINUNIX)
	-test -e hydra.exe && strip $(BINWIN) || touch hydra
	-test -e xhydra && strip xhydra || touch Makefile

install:	strip
	-test -e hydra.exe && touch Makefile || cp $(BINUNIX) $(PREFIX)$(DIR) && cd $(PREFIX)$(DIR) && chmod 755 $(BINUNIX)
	-test -e hydra.exe && cp $(BINWIN) $(PREFIX)$(DIR) && cd $(PREFIX)$(DIR) && chmod 755 $(BINWIN) || touch Makefile
	-test -e xhydra && cp xhydra $(PREFIX)$(DIR) && cd $(PREFIX)$(DIR) && chmod 755 xhydra

clean:
	rm -rf hydra pw-inspector xhydra pw-inspector.exe hydra.exe *.o core *~ Makefile.in Makefile dev_rfc hydra.restore arm/*.ipk arm/ipkg/usr/bin/* hydra-gtk/src/*.o hydra-gtk/src/xhydra hydra-gtk/stamp-h hydra-gtk/config.status hydra-gtk/errors hydra-gtk/config.log hydra-gtk/src/.deps hydra-gtk/src/Makefile hydra-gtk/Makefile

backup:
	tar czvf ../hydra-dev.tgz *.c *.h README Makefile.am configure TODO CHANGES INSTALL LICENCE.HYDRA LICENSE.GNU

revise:
	mv ../hydra-dev.tgz ../hydra-dev-old.tgz

striparm: hydra pw-inspector
	$(STRIP) hydra
	$(STRIP) pw-inspector

buildipkg: striparm hydra pw-inspector
	cp hydra pw-inspector arm/ipkg/usr/bin/
	(cd arm/;./ipkg-build ipkg)
