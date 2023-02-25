# @(#)86        1.2  src/bos/usr/sbin/devinstall/Makefile, cmdcfg, bos411, 9428A410j 12/13/93 10:47:59
#
# COMPONENT_NAME: (CMDCFG)
#
# FUNCTIONS: makefile for devinstall command
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

VPATH		= ${TARGET_MACHINE}

PROGRAMS	= devinstall
MSGHDRS		=
CATFILES	= devinstall.cat
OTHERS		= ${CATFILES}

ILIST		= devinstall
IDIR		= /usr/sbin/

OFILES		= devinstall.o

LIBS		= -ls -lodm -lcfg

.include <${RULES_MK}>

