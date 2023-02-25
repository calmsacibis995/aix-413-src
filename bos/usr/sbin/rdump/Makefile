# @(#)99        1.4  src/bos/usr/sbin/rdump/Makefile, cmdarch, bos411, 9428A410j 8/27/93 11:07:31
#
# COMPONENT_NAME: CMDARCH
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= rdump backbyinode
OTHERS		= dumpdates
MSGHDRS		= dumpi_msg.h
CATFILES	= dumpi.cat
HFILES		= ${MSGHDRS}

ILIST		= ${PROGRAMS} ${OTHERS}
IDIR		= /usr/sbin/
LIBS		= -lfs
dumpdates_IDIR	= /etc/

rdump_OFILES	= dumpitime.o dumprmain.o dumproptr.o dumprmt.o \
		  dumprtape.o dumprtravrs.o unctime.o

backbyinode_OFILES	= dumpitime.o dumpmain.o dumpoptr.o dumptape.o \
			  dumptravrs.o unctime.o

dumprmain.o_CFLAGS	= -DRDUMP
dumprtravrs.o_CFLAGS	= -DRDUMP
dumprtape.o_CFLAGS	= -DRDUMP
dumproptr.o_CFLAGS	= -DRDUMP

.include <${RULES_MK}>

dumprmain.c	: dumpmain.c
	${CP} ${dumpmain.c:P} ${.TARGET}

dumprtravrs.c	: dumptravrs.c
	${CP} ${dumptravrs.c:P} ${.TARGET}

dumproptr.c	: dumpoptr.c
	${CP} ${dumpoptr.c:P} ${.TARGET}

dumpdates:
	${TOUCH} ${.TARGET}
