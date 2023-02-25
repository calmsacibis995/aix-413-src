# @(#)93        1.2  src/bos/usr/sbin/diskusg/Makefile, cmdfs, bos411, 9428A410j 11/19/92 18:42:24
#
# COMPONENT_NAME: (CMDFS) commands that deal with the file system
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1992
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= diskusg
MSGHDRS		= diskusg_msg.h
CATFILES	= diskusg.cat
HFILES		= ${MSGHDRS}

ILIST		= diskusg
IDIR		= /usr/sbin/
LIBS		= -lfs

#
# for dbx debugging
#
#CC_OPT_LEVEL	= -g
#.include <${RULES_MK}>
#_LDFLAGS_PROG_  = -b f -bh:4 -bi -D0 -H512 -b 'loadmap:${.TARGET}.map' -bR:${.TARGET}.rmap

#
# For no debug
#
.include <${RULES_MK}>
