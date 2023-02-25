# @(#)96        1.3  src/bos/usr/sbin/dumpfs/Makefile, cmdfs, bos410, bos4.17293n 11/19/92 17:58:22
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

PROGRAMS	= defragfs
MSGHDRS		= defragfs_msg.h
CATFILES	= defragfs.cat
HFILES		= ${MSGHDRS}

ILIST		= defragfs
IDIR		= /usr/sbin/


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

