# @(#)45        1.3  src/bos/usr/sbin/tsm/Makefile, Makefiles, bos411, 9428A410j 7/27/93 12:35:41
# COMPONENT_NAME: CMDSAUTH
#
# FUNCTIONS: NONE
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
#
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= tsm
MSGHDRS		= tsm_msg.h
CATFILES	= tsm.cat
HFILES		= ${MSGHDRS}

ILIST		= tsm
IDIR		= /usr/sbin/

OFILES		= tsm.o tsmports.o tsmerr.o tsmlogin.o tsmgetty.o tsm_su_util.o

LIBS		= -ls -lPW -lodm -lcfg

.include <${RULES_MK}>
