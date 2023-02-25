# @(#)87        1.2  src/bos/usr/ucb/vacation/Makefile, cmdmailx, bos411, 9428A410j 1/21/93 18:04:18
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

PROGRAMS	= vacation
MSGHDRS		= vac_msg.h
CATFILES	= vac.cat
HFILES		= ${MSGHDRS}

ILIST		= vacation
IDIR		= /usr/bin/

CFLAGS		= -D_BSD -DAIX -DVMUNIX -DV7 -DBSD_INCLUDES -DNOCATMACRO

LIBS		= -ldbm

.include <${RULES_MK}>
