# @(#)32	1.2  src/tcpip/usr/lbin/fingerd/Makefile, tcpadmin, tcpip411, GOLD410 1/18/93 11:33:19
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

PROGRAMS                = fingerd
MSGHDRS                 = fingerd_msg.h
CATFILES                = fingerd.cat

IDIR                    = /usr/sbin/
ILIST                   = fingerd

CCTYPE                  = writable_strings
CFLAGS                  = -D_BSD=44
HFILES                  = ${MSGHDRS}
LIBS                    = -lbsd

.include <${RULES_MK}>
