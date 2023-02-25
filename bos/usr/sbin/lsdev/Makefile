# @(#)35        1.2  src/bos/usr/sbin/lsdev/Makefile, cfgmethods, bos411, 9428A410j 3/24/93 10:21:55
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

VPATH		= ../mkdev

PROGRAMS	= lsdev
MSGHDRS		=
CATFILES	= cmdcfg.cat
HFILES		= ${MSGHDRS}

ILIST		= lsdev
IDIR		= /usr/sbin/

OFILES		= lsdev.o lscf.o lscfprt.o cfglib.o defaultmsg.o

LIBS		= -ls -lodm -lcfg

.include <${RULES_MK}>
