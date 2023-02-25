# @(#)03	1.2  src/tcpip/usr/sbin/rwhod/Makefile, tcpadmin, tcpip411, GOLD410 4/28/94 17:01:02
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

PROGRAMS                = rwhod
MSGHDRS                 = rwhod_msg.h
CATFILES                = rwhod.cat

IDIR                    = /usr/sbin/
ILIST                   = rwhod

CCTYPE                  = writable_strings
CFLAGS                  = -D_BSD=44
HFILES                  = ${MSGHDRS}
LIBS                    = -lbsd -lsrc

.include <${RULES_MK}>
