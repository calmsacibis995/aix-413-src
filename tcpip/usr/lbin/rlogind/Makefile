# @(#)35	1.4  src/tcpip/usr/lbin/rlogind/Makefile, tcprcmds, tcpip411, GOLD410 4/5/94 17:53:57
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

PROGRAMS                = rlogind
MSGHDRS                 = rlogind_msg.h
CATFILES                = rlogind.cat

IDIR                    = /usr/sbin/
ILIST                   = rlogind

CCTYPE                  = writable_strings
CFLAGS                  = -D_BSD=44 
HFILES                  = ${MSGHDRS}
OFILES                  = rlogind.o release_license.o
LIBS                    = -lbsd -ltcp_ocs -ls -lodm -lcfg

.include <${RULES_MK}>
