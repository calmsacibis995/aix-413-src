# @(#)64        1.1  src/bos/usr/sbin/auditbin/Makefile, cmdsaud, bos411, 9428A410j 8/5/92 17:39:59
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

PROGRAMS	= auditbin
MSGHDRS		= auditbin_msg.h
CATFILES	= auditbin.cat
HFILES		= ${MSGHDRS}

ILIST		= auditbin
IDIR		= /usr/sbin/

INCFLAGS	= -I../audit

OFILES		= auditbin.o audutil.o cfgcutil.o

LIBS		= -ls

.include <${RULES_MK}>
