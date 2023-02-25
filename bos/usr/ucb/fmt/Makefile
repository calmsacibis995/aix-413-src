# @(#)86        1.2  src/bos/usr/ucb/fmt/Makefile, cmdmailx, bos411, 9428A410j 1/21/93 18:07:18
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

VPATH		= ${MAKETOP}/bos/usr/bin/mail

PROGRAMS	= fmt
MSGHDRS		= mail_msg.h
CATFILES	=
HFILES		= ${MSGHDRS}

ILIST		= fmt
IDIR		= /usr/bin/

CFLAGS		= -D_BSD -DAIX -DVMUNIX -DV7 -DBSD_INCLUDES -DNOCATMACRO

OFILES		= fmt.o head.o

.include <${RULES_MK}>
