# @(#)57	1.2  src/tcpip/usr/bin/talk/Makefile, tcptalk, tcpip411, GOLD410 2/21/94 08:44:16
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

PROGRAMS                = talk
MSGHDRS                 = talk_msg.h
CATFILES                = talk.cat

IDIR                    = /usr/bin/
ILIST                   = talk

CCTYPE                  = writable_strings
CFLAGS                  = -DS5 -D_BSD=44 ${_AIX3CURFLAGS}
LIBFLAGS		+= ${_AIX3CURLIBFLAG}

HFILES                  = ${MSGHDRS} talk.h talk_ctl.h
OFILES                  = talk.o get_names.o display.o io.o ctl.o init_disp.o \
			msgs.o get_addrs.o ctl_trans.o invite.o look_up.o
LIBS 			= -lbsd -lcur

.include <${RULES_MK}>
