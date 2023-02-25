# @(#)52	1.2  src/tcpip/usr/bin/nslookup/Makefile, tcpras, tcpip411, GOLD410 6/3/93 11:24:50
#
#   COMPONENT_NAME: TCPIP
#
#   FUNCTIONS: none
#
#   ORIGINS: 26 27 65
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
# ALL RIGHTS RESERVED 
#
#

PROGRAMS		= nslookup
MSGHDRS			= nslookup_msg.h
CATFILES		= nslookup.cat

ILIST			= nslookup
IDIR			= /usr/bin/

CCTYPE			= writable_strings
CFLAGS			= -D_BSD=44
#LFLAGS			= -I
HFILES			= nslookup_msg.h res.h
OFILES			= getinfo.o commands.o main.o skip.o debug.o list.o \
			  send.o subr.o 
#LIBS			= -lbsd -ll -lresolv
LIBS			= -lbsd -ll
GARBAGE			= commands.c

.include <${RULES_MK}>
