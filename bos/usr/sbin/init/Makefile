# @(#)33        1.3  src/bos/usr/sbin/init/Makefile, cmdoper, bos411, 9428A410j 10/27/93 14:53:48
#
# COMPONENT_NAME: CMDOPER
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= init
MSGHDRS		= init_msg.h
CATFILES	= init.cat
HFILES		= ${MSGHDRS}

ILIST		= init
IDIR		= /usr/sbin/

OFILES		= init.o getcmd.o signals.o output.o

LIBS		= -ls -lrts

.include <${RULES_MK}>
