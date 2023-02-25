# @(#)58	1.3  src/tcpip/usr/bin/telnet/Makefile, tcptelnet, tcpip411, GOLD410 2/21/94 08:44:23
# 
# COMPONENT_NAME: TCPIP Makefile
# 
# FUNCTIONS: 
#
# ORIGINS: 10  26  27 
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#

PROGRAMS                = telnet
MSGHDRS                 = telnet_msg.h
CATFILES                = telnet.cat

IDIR                    = /usr/bin/
ILIST                   = telnet

CCTYPE                  = writable_strings

CFLAGS                  = -DAIX -D_BSD=44 -D_CSECURITY -DARAB -DBIDI \
			${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS		+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

HFILES                  = ${MSGHDRS}
OFILES                  = telnet.o vtif.o vtparse.o fs3270.o profile.o \
			tables.o scan.o teltoterm.o lookup.o map3270.o
LIBS			= -lbsd -lc -lcur -lcurses -ls -liconv -ltcp_audit \
			-li18n


.include <${RULES_MK}>
