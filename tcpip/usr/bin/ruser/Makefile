# @(#)63	1.1  src/tcpip/usr/bin/ruser/Makefile, tcprcmds, tcpip411, GOLD410 12/8/92 11:50:53
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

PROGRAMS                = ruser
MSGHDRS                 = ruser_msg.h
CATFILES                = ruser.cat

IDIR                    = /usr/bin/
ILIST                   = ruser

CCTYPE                  = writable_strings
CFLAGS                  = -D_BSD=44
HFILES                  = ${MSGHDRS}
LIBS                    = -lffdb -lbsd

.include <${RULES_MK}>
