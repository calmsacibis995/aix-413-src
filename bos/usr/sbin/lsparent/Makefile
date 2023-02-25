# @(#)37        1.1  src/bos/usr/sbin/lsparent/Makefile, cfgmethods, bos411, 9428A410j 8/31/92 19:19:32
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

VPATH		= ../mkdev:../lsdev

PROGRAMS	= lsparent
MSGHDRS		= 
CATFILES	= 
HFILES		= ${MSGHDRS}

ILIST		= lsparent
IDIR		= /usr/sbin/

INCFLAGS	= -I../lsdev

OFILES		= lsparent.o lscf.o lscfprt.o cfglib.o defaultmsg.o

LIBS		= -ls -lodm -lcfg

.include <${RULES_MK}>
