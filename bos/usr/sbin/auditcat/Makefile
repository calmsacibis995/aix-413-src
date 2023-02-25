# @(#)65        1.1  src/bos/usr/sbin/auditcat/Makefile, cmdsaud, bos411, 9428A410j 8/5/92 17:40:48
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

PROGRAMS	= auditcat
MSGHDRS		= auditcat_msg.h
CATFILES	= auditcat.cat
HFILES		= ${MSGHDRS}

ILIST		= auditcat
IDIR		= /usr/sbin/

INCFLAGS	= -I../audit

OFILES		= auditcat.o audutil.o cfgcutil.o

LIBS		= -ls

.include <${RULES_MK}>
