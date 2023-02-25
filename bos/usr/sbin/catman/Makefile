# @(#)90        1.2  src/bos/usr/sbin/catman/Makefile, cmdman, bos411, 9428A410j 10/19/93 17:18:51
# COMPONENT_NAME: (CMDMAN) commands that allow users to read online
# documentation
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

PROGRAMS	= catman
MSGHDRS		= catman_msg.h
CATFILES	= catman.cat
HFILES		= ${MSGHDRS}

OFILES		= catman.o whatis.o
CFLAGS		= -DKRS_API -DMSG

LIBS		= -lori2

ILIST		= catman
IDIR		= /usr/sbin/

.include <${RULES_MK}>
