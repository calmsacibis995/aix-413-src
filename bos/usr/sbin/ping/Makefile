# @(#)74        1.2  src/bos/usr/sbin/ping/Makefile, cmdnet, bos411, 9428A410j 9/20/93 16:53:01
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

PROGRAMS	= ping
MSGHDRS		= ping_msg.h
CATFILES	= ping.cat
HFILES		= ${MSGHDRS}

ILIST		= ping
IDIR		= /usr/sbin/

CFLAGS		= -D_BSD=44 -DMAXLOGINS=16 -DIP_MULTICAST

.include <${RULES_MK}>
