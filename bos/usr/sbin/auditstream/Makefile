# @(#)62        1.1  src/bos/usr/sbin/auditstream/Makefile, cmdsaud, bos411, 9428A410j 8/5/92 17:37:44
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

VPATH		= ../audit

PROGRAMS	= auditstream
MSGHDRS		= auditstm_msg.h
CATFILES	= auditstm.cat
HFILES		= ${MSGHDRS}

ILIST		= auditstream
IDIR		= /usr/sbin/

INCFLAGS	= -I../audit

OFILES		= auditstream.o audutil.o

LIBS		= -ls

.include <${RULES_MK}>
