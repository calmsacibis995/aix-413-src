# @(#)37	1.2  src/tcpip/usr/lbin/talkd/Makefile, tcptalk, tcpip411, GOLD410 1/18/93 11:45:10
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

PROGRAMS                = talkd
MSGHDRS                 = talkd_msg.h
CATFILES                = talkd.cat

IDIR                    = /usr/sbin/
ILIST                   = talkd

CCTYPE                  = writable_strings
CFLAGS                  = -DS5  -D_BSD=44
HFILES                  = ${MSGHDRS}
OFILES                  = talkd.o announce.o process.o table.o print.o
LIBS                    = -lbsd

.include <${RULES_MK}>
