# @(#)04	1.2  src/tcpip/usr/sbin/timed/Makefile, tcptimer, tcpip411, GOLD410 2/11/94 14:13:07
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

PROGRAMS                = timed timedc
MSGHDRS                 = timed_msg.h
CATFILES                = timed.cat

IDIR                    = /usr/sbin/
ILIST                   = timed timedc

CCTYPE                  = writable_strings
CFLAGS                  = -D_BSD=44 -DMEASURE -DTESTING
HFILES                  = ${MSGHDRS} globals.h timedc.h

timed_OFILES            = ${OBJDS} ${COMMO}
timedc_OFILES           = ${OBJCS} ${COMMO}

OBJDS = acksend.o candidate.o correct.o master.o networkdelta.o readmsg.o \
        slave.o timed.o logwtmp.o
OBJCS = cmds.o cmdtab.o timedc.o
COMMO = byteorder.o measure.o

LIBS                    = -lbsd

.include <${RULES_MK}>
