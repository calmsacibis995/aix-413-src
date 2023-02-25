# @(#)66	1.1  src/tcpip/usr/bin/rexec/Makefile, tcprcmds, tcpip411, GOLD410 12/8/92 11:51:33
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

PROGRAMS                = rexec
MSGHDRS                 = rexec_msg.h
CATFILES                = rexec.cat

IDIR                    = /usr/bin/
ILIST                   = rexec

CCTYPE                  = writable_strings
CFLAGS                  = -D_BSD=44 -DSDB -DMANYGROUPS -DS5 -D_CSECURITY
HFILES                  = ${MSGHDRS}
LIBS 			= -lbsd
LIBS			= -lbsd -ls -ltcp_audit

.include <${RULES_MK}>
