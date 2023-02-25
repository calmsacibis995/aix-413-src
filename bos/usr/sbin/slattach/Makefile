# @(#)70        1.2  src/bos/usr/sbin/slattach/Makefile, cmdnet, bos411, 9428A410j 7/23/93 09:24:13
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

VPATH		= ${MAKETOP}/bos/usr/bin/uucp

PROGRAMS	= slattach
MSGHDRS		= slattach_msg.h
CATFILES	= slattach.cat
HFILES		= ${MSGHDRS}

ILIST		= slattach
IDIR		= /usr/sbin/

CFLAGS		= -D_BSD=44 -DMAXLOGINS=16

INCFLAGS	= -I${MAKETOP}/bos/usr/bin/uucp

OFILES		= slattach.o scallers.o sconn.o getargs.o line.o \
		  ulockf.o altconn.o uucpdefs.o interface.o sysfiles.o \
		  strsave.o

scallers.o_CFLAGS	= -DSTANDALONE
sconn.o_CFLAGS		= -DSTANDALONE


.include <${RULES_MK}>
