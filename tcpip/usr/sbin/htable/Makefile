# @(#)31	1.3  src/tcpip/usr/sbin/htable/Makefile, tcpadmin, tcpip411, GOLD410 2/11/94 13:30:48
# 
# COMPONENT_NAME: TCPIP Makefile
# 
# FUNCTIONS: 
#
# ORIGINS: 10  26  27 
#
# (C) COPYRIGHT International Business Machines Corp. 1992
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS                = htable gettable
MSGHDRS                 = htable_msg.h gettable_msg.h
CATFILES                = htable.cat gettable.cat

IDIR                    = /usr/sbin/
ILIST                   = htable gettable

CCTYPE                  = writable_strings
CFLAGS                  = -D_BSD=44
YFLAGS                  = -d
HFILES                  = ${MSGHDRS}
htable_OFILES           = htable.o parse.o scan.o
gettable_OFILES         = gettable.o
LIBS                    = -lbsd

.include <${RULES_MK}>
