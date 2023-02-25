# @(#)51	1.1  src/tcpip/usr/bin/ftp/Makefile, tcpfilexfr, tcpip411, GOLD410 12/8/92 11:48:26
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

PROGRAMS                = ftp
MSGHDRS                 = ftp_msg.h
CATFILES                = ftp.cat

IDIR                    = /usr/bin/
ILIST                   = ftp

CCTYPE                  = writable_strings
CFLAGS                  = -D_CSECURITY -D_BSD=44
HFILES                  = ${MSGHDRS}
OFILES                  = cmds.o cmdtab.o ftp.o	main.o pclose.o	ruserpass.o \
			domacro.o
LIBS 			= -ltcp_audit -ls -lbsd -lrts

.include <${RULES_MK}>
