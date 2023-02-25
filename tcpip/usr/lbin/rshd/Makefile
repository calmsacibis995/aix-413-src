# @(#)36	1.2  src/tcpip/usr/lbin/rshd/Makefile, tcprcmds, tcpip411, GOLD410 1/18/93 11:42:43
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

PROGRAMS                = rshd
MSGHDRS                 = rshd_msg.h
CATFILES                = rshd.cat

IDIR                    = /usr/sbin/
ILIST                   = rshd

CCTYPE                  = writable_strings
CFLAGS                  = -D_BSD=44 
HFILES                  = ${MSGHDRS}
LIBS                    = -lbsd -ls

.include <${RULES_MK}>
