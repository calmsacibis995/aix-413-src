# @(#)95        1.2  src/bos/usr/sbin/ncheck/Makefile, cmdfs, bos411, 9428A410j 2/23/93 10:01:03
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

PROGRAMS	= ncheck
MSGHDRS		= ncheck_msg.h
CATFILES	= ncheck.cat
HFILES		= ${MSGHDRS}

ILIST		= ncheck
IDIR		= /usr/sbin/

LIBS		= -lIN -lfs

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
