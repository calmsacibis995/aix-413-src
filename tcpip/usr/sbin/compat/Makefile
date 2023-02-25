# @(#)13	1.1  src/tcpip/usr/sbin/compat/Makefile, tcpip, tcpip411, GOLD410 3/30/94 17:20:38
# 
# COMPONENT_NAME: TCPIP Makefile
# 
# FUNCTIONS: 
#
# ORIGINS: 27 
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS                = inetserv inetimp inetexp stinet notinet
MSGHDRS                 = inetserv_msg.h inetimp_msg.h inetexp_msg.h \
			  stinet_msg.h notinet_msg.h
CATFILES                = inetserv.cat inetimp.cat inetexp.cat \
			  stinet.cat notinet.cat

IDIR                    = /usr/sbin/
ILIST                   = inetserv inetimp inetexp stinet notinet

CCTYPE                  = writable_strings

inetserv_CFILES		= inetserv.c inetodm.c
inetimp_CFILES		= inetimp.c inetodm.c
inetexp_CFILES		= inetexp.c inetodm.c
stinet_CFILES		= stinet.c inetodm.c
notinet_CFILES		= notinet.c

inetserv_OFILES		= inetserv.o inetodm.o
inetimp_OFILES		= inetimp.o inetodm.o
inetexp_OFILES		= inetexp.o inetodm.o
stinet_OFILES		= stinet.o inetodm.o
notinet_OFILES		= notinet.o

HFILES                  = ${MSGHDRS}
LIBS                    = -lffdb -lodm -lbsd

.include <${RULES_MK}>
