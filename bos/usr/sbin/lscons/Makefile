# @(#)98        1.2  src/bos/usr/sbin/lscons/Makefile, cfgmethods, bos411, 9428A410j 1/25/93 11:25:13
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

VPATH		= ${MAKETOP}/bos/usr/lib/methods/common

PROGRAMS	= lscons
MSGHDRS		= console_msg.h
HFILES		= ${MSGHDRS}

INCFLAGS	= -I../common

ILIST		= lscons
IDIR		= /usr/sbin/

LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
