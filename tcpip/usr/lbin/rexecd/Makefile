# @(#)34	1.2  src/tcpip/usr/lbin/rexecd/Makefile, tcprcmds, tcpip411, GOLD410 1/18/93 11:37:30
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

PROGRAMS                = rexecd
MSGHDRS                 = rexecd_msg.h
CATFILES                = rexecd.cat

IDIR                    = /usr/sbin/
ILIST                   = rexecd

CCTYPE                  = writable_strings
CFLAGS                  = -D_BSD=44 -D_CSECURITY
HFILES                  = ${MSGHDRS}
LIBS                    = -lbsd -ls -ltcp_audit

.include <${RULES_MK}>
