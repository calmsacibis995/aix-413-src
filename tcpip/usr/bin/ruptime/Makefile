# @(#)55	1.1  src/tcpip/usr/bin/ruptime/Makefile, tcpadmin, tcpip411, GOLD410 12/8/92 11:49:13
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

PROGRAMS                = ruptime
MSGHDRS                 = ruptime_msg.h
CATFILES                = ruptime.cat

IDIR                    = /usr/bin/
ILIST                   = ruptime

CCTYPE                  = writable_strings
CFLAGS                  = -D_BSD=44 -DSDB -DMANYGROUPS -DS5 -D_CSECURITY
HFILES                  = ${MSGHDRS}
LIBS 			= -lbsd

.include <${RULES_MK}>
