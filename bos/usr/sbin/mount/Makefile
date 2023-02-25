# @(#)92        1.2  src/bos/usr/sbin/mount/Makefile, cmdfs, bos411, 9428A410j 6/17/93 12:14:06
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

PROGRAMS	= mount
MSGHDRS		= mount_msg.h
CATFILES	= mount.cat
HFILES		= ${MSGHDRS}

ILIST		= mount
IDIR		= /usr/sbin/

LIBS		= -lfs -lIN

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
