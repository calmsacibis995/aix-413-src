# @(#)30        1.2  src/bos/usr/sbin/mkdev/Makefile, cmdcfg, bos411, 9428A410j 11/23/92 13:59:33
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= mkdev
MSGHDRS		=
CATFILES	=
HFILES		= ${MSGHDRS}

ILIST		= mkdev
IDIR		= /usr/sbin/

INCLUDES	= hlcmds.h
hlcmds.h_IDIR	= /usr/include/

OFILES		= mkdev.o cfglib.o defaultmsg.o

LIBS		= -ls -lodm -lcfg

.include <${RULES_MK}>
