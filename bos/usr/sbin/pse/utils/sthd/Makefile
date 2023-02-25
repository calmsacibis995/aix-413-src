# @(#)53        1.3  src/bos/usr/sbin/pse/utils/sthd/Makefile, Makefiles, bos411, 9428A410j 8/31/93 02:57:53
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27 83
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
# LEVEL 1, 5 Years Bull Confidential Information

VPATH		= ${MAKETOP}/bos/kernext/exp

PROGRAMS	= sthd
MSGHDRS		= 
CATFILES	= 
HFILES		= ${MSGHDRS}

gpmsgt.o_CFLAGS = -DEARLY_ADDR_CHECK
gppmsgt.o_CFLAGS= -DEARLY_ADDR_CHECK

OFILES		= atmarkt.o autot.o bandt.o closewt.o fdint.o findt.o \
		  gpmsgt.o gppmsgt.o linkt.o listt.o lookt.o nreadt.o \
		  peekt.o plinkt.o pollt.o printe.o pusht.o rdoptt.o \
		  seekt.o setoptst.o sigt.o srfdt.o sthd.o strt.o

sthd_IMPORTS	= -bI:pse.exp

LIBS		= -lmi -ltli

.include <${RULES_MK}>
