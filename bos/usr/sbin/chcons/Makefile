# @(#)78        1.3  src/bos/usr/sbin/chcons/Makefile, cfgmethods, bos411, 9428A410j 10/21/93 13:20:24
#
#   COMPONENT_NAME: CFGMETHODS
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1988,1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

VPATH		= ${MAKETOP}/bos/usr/lib/methods/common

PROGRAMS	= chcons
MSGHDRS		= console_msg.h
HFILES		= ${MSGHDRS}

OFILES		= chcons.o

ILIST		= chcons
IDIR		= /usr/sbin/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
