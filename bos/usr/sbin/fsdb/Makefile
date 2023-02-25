# @(#)94        1.3  src/bos/usr/sbin/fsdb/Makefile, cmdfs, bos411, 9428A410j 3/3/93 08:58:43
# COMPONENT_NAME:
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
PROGRAMS	= fsdb
MSGHDRS		= fsdb_msg.h
CATFILES	= fsdb.cat
HFILES		= ${MSGHDRS}

ILIST		= fsdb
IDIR		= /usr/sbin/

OFILES		= fsdb.o fsdb_read.o fsdb_dir.o

LIBS		= -lIN -lfs

#
# for dbx debugging
#
#CC_OPT_LEVEL	= -g
#.include <${RULES_MK}>
#_LDFLAGS_PROG_  = -b f -bh:4 -bi -D0 -H512 -b 'loadmap:${.TARGET}.map' -bR:${.TARGET}.rmap
#


#
# For no debug
#
.include <${RULES_MK}>
