# @(#)04        1.2.1.1  src/bos/usr/sbin/execerror/Makefile, cmdcntl, bos41J, 9507C 2/8/95 08:10:15
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= execerror
MSGHDRS		= execerr_msg.h
CATFILES	= execerr.cat
HFILES		= ${MSGHDRS}

ILIST		= execerror
IDIR		= /usr/sbin/

VPATH			= ${MAKETOP}/bos/kernel/exp
execerror_IMPORTS	= -bI:syscalls.exp
execerror_LDFLAGS	= -bnso

.include <${RULES_MK}>
