# @(#)38	1.5  src/tcpip/usr/lbin/telnetd/Makefile, tcptelnet, tcpip411, GOLD410 4/5/94 17:47:15
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

PROGRAMS                = telnetd
MSGHDRS                 = telnetd_msg.h
CATFILES                = telnetd.cat

IDIR                    = /usr/sbin/
ILIST                   = telnetd

CCTYPE                  = writable_strings
CFLAGS                  = -DS5  -D_BSD=44 -D_CSECURITY
HFILES                  = ${MSGHDRS}
OFILES                  = telnetd.o teltoterm.o release_license.o
LIBS                    = -lbsd -ltcp_ocs -ltcp_audit -ls -lodm -lcfg

.include <${RULES_MK}>
