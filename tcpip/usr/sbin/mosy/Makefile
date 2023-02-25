# @(#)30      1.3  src/tcpip/usr/sbin/mosy/Makefile, tcpip, tcpip410 2/22/93 17:05:25
#
# COMPONENT_NAME:  (ISODELIB7) ISODE Libraries, Release 7
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1993
# All Rights Reserved
# Licensed Material - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}tcpip/usr/Make.cfg>

#LOCALCFLAGS=-v -qlist

VPATH=${MAKETOP}tcpip/usr/lib/libisode

PROGRAMS                = mosy

IDIR                    = /usr/sbin/
ILIST                   = mosy

CCTYPE                  = writable_strings
OFILES                  = mosy.o mosy_dump.o pepy_misc.o yacc.o mosyvrsn.o
HFILES			= mosy-defs.h pepy.h

LIBS                    = -lsnmp -lisode -lm

.include <${RULES_MK}>

mosyvrsn.c: version.sh
	${CP} ${version.sh:P} version.sh
	${CHMOD} +x version.sh
	version.sh mosy > $@
	${RM} -f version.sh

yacc.o:	yacc.c lex.c

yacc.c:	yacc.y yacc.y.gnrc

yacc.y: yacc.y.gnrc extract.sh
	${RM} -f extract.sh
	${CP} ${extract.sh:P} extract.sh
	${CHMOD} +x extract.sh
	extract.sh MOSY < ${yacc.y.gnrc:P} > $@
	${RM} -f extract.sh

lex.c: lex.l lex.l.gnrc

lex.l: lex.l.gnrc extract.sh
	${RM} -f extract.sh
	${CP} ${extract.sh:P} extract.sh
	${CHMOD} +x extract.sh
	extract.sh MOSY < ${lex.l.gnrc:P} > $@
	${RM} -f extract.sh

mosy.o: mosy-defs.h

pepy_misc.o: pepy.h

mosy-defs.h: pepsy.h.gnrc extract.sh
	${RM} -f extract.sh
	${CP} ${extract.sh:P} extract.sh
	${CHMOD} +x extract.sh
	extract.sh MOSY < ${pepsy.h.gnrc:P} > $@
	${RM} -f extract.sh

pepy.h: pepsy.h.gnrc extract.sh
	${RM} -f extract.sh
	${CP} ${extract.sh:P} extract.sh
	${CHMOD} +x extract.sh
	extract.sh PEPY < ${pepsy.h.gnrc:P} > $@
	${RM} -f extract.sh

