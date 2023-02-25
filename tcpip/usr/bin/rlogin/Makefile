# @(#)53	1.1  src/tcpip/usr/bin/rlogin/Makefile, tcprcmds, tcpip411, GOLD410 12/8/92 11:48:51
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

PROGRAMS                = rlogin
MSGHDRS                 = rlogin_msg.h
CATFILES                = rlogin.cat

IDIR                    = /usr/bin/
ILIST                   = rlogin

CCTYPE                  = writable_strings
CFLAGS                  = -D_BSD=44 -DSDB -DMANYGROUPS -DS5 -D_CSECURITY
HFILES                  = ${MSGHDRS}
LIBS 			= -lbsd

.include <${RULES_MK}>
