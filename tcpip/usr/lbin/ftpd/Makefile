# @(#)33        1.5  src/tcpip/usr/lbin/ftpd/Makefile, tcpfilexfr, tcpip41J, 9516A_all 4/17/95 14:16:50
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

PROGRAMS                = ftpd
SCRIPTS			= newvers
MSGHDRS                 = ftpd_msg.h
CATFILES                = ftpd.cat

IDIR                    = /usr/sbin/
ILIST                   = ftpd

CCTYPE                  = writable_strings
CFLAGS                  = -D_ANSI -D_CSECURITY -D_BSD=44 -Dunix
YFLAGS                  = -d
HFILES                  = ${MSGHDRS}
OFILES                  = ftpd.o ftpcmd.o popen.o logwtmp.o vers.o
LIBS                    = -lbsd -ls -ltcp_audit

.include <${RULES_MK}>

vers.c: ftpd.c ftpcmd.y newvers
	${RM} -f ${.TARGET} version
	./newvers || { ${RM} -f ${.TARGET}; ${FALSE}; }
