# @(#)02	1.1  src/tcpip/usr/sbin/routed/Makefile, tcprouting, tcpip411, GOLD410 12/8/92 11:39:55
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

PROGRAMS                = routed
MSGHDRS                 = routed_msg.h
CATFILES                = routed.cat

IDIR                    = /usr/sbin/
ILIST                   = routed

CCTYPE                  = writable_strings
CFLAGS                  = -DS5 -D_BSD=44
HFILES                  = ${MSGHDRS}
OFILES                  = af.o if.o inet.o input.o main.o output.o startup.o \
			tables.o timer.o trace.o
LIBS                    = -lbsd -lsrc

.include <${RULES_MK}>
