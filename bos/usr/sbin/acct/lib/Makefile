# @(#)36        1.1  src/bos/usr/sbin/acct/lib/Makefile, cmdacct, bos411, 9428A410j 8/5/92 17:58:01
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

VPATH		= ..

LIBRARIES	= liba.a
MSGHDRS		= acct_msg.h
CATFILES	= 
HFILES		= ${MSGHDRS}

INCFLAGS	= -I..

OFILES		= cat.o copyn.o devtolin.o eprintf.o expand.o extend.o \
		  hashuser.o lintodev.o namtouid.o pnpsplit.o regex.o \
		  regcmp.o substr.o tmless.o tmsecs.o uidtonam.o

.include <${RULES_MK}>
