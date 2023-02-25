# @(#)00        1.2  src/bos/usr/sbin/rrestore/Makefile, cmdarch, bos411, 9428A410j 2/18/93 10:58:50
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

VPATH		= ../rdump

PROGRAMS	= rrestore restbyinode
MSGHDRS		= restore2_msg.h
CATFILES	= restore2.cat
HFILES		= ${MSGHDRS}

ILIST		= rrestore restbyinode
IDIR		= /usr/sbin/

restbyinode_OFILES	= main.o interactiv.o restore.o dirs.o symtab.o \
			  utilities.o tape.o

rrestore_OFILES		= rmain.o interactiv.o restore.o dirs.o symtab.o \
			  utilities.o rtape.o dumprmt.o

rtape.o_CFLAGS	= -DRRESTORE
rmain.o_CFLAGS	= -DRRESTORE
dumprmt.o_CFLAGS	= -DRRESTORE

dumprmt.o_INCFLAGS	= -I../rdump

.include <${RULES_MK}>

rmain.c		: main.c
	${CP} ${main.c:P} ${.TARGET}

rtape.c		: tape.c
	${CP} ${tape.c:P} ${.TARGET}

