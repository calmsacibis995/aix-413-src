# @(#)62	1.1  src/tcpip/usr/bin/setclock/Makefile, tcptimer, tcpip411, GOLD410 12/8/92 11:50:39
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

PROGRAMS                = setclock
MSGHDRS                 = setclock_msg.h
CATFILES                = setclock.cat

IDIR                    = /usr/bin/
ILIST                   = setclock

CCTYPE                  = writable_strings
HFILES                  = ${MSGHDRS}
CFLAGS			= -D_CSECURITY
LIBS			= -ls -ltcp_audit

.include <${RULES_MK}>
