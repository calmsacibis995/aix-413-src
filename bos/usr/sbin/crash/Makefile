# @(#)29        1.10  src/bos/usr/sbin/crash/Makefile, cmdcrash, bos411, 9435A411a 8/25/94 17:07:51
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# LEVEL 1,5 Years Bull Confidential Information
#

VPATH		= ${TARGET_MACHINE}

PROGRAMS	= crash
MSGHDRS		= crash_msg.h
CATFILES	= crash.cat
HFILES		= ${MSGHDRS}

ILIST		= crash
IDIR		= /usr/sbin/

CFLAGS		= -D_KERNEL

printu.o_CFLAGS	= -D_KERNEL -U__MATH__
dla.o_CFLAGS 	= -D_USER -D_CRASH -D_KERNEL


OFILES		= \
	main.o symtab.o sysvad.o file.o callout.o stat.o inode.o proc.o \
	printu.o u.o prvfs.o prvnode.o mbuf.o socket.o xmalloc.o buf.o \
	dump.o fstack.o xdumpx.o chngmap.o pstat.o tty.o printmst.o \
	printle.o misc.o decode.o isearch.o errpt.o stab.o ndb.o \
	thread.o thrdutil.o mp.o dla.o streams.o symptom.o

LIBS	= -lrts

.include <${RULES_MK}>
