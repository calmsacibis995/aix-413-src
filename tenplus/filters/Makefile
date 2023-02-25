# @(#)74	1.8  src/tenplus/filters/Makefile, tenplus, tenplus411, GOLD410 6/8/93 13:30:08
# Makefile tenplus/filters/Makefile
# COMPONENT_NAME: (INED) INed Editor
# 
# ORIGINS:  9, 10, 27
# 
# This module contains IBM CONFIDENTIAL code. -- (IBM
# Confidential Restricted when combined with the aggregated
# modules for this product)                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1985, 1993
# All Rights Reserved
# 
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.


PROGRAMS = fjust rpl fformat just
MSGHDRS = rpl_msg.h
CATFILES = rpl.cat
HFILES = ${MSGHDRS}
INCFLAGS = -I${MAKETOP}/tenplus/include

OFILES =  fjust.o rpl.o fformat.o fcommand.o fiospace.o fparser.o  \
          flexer.o foutput.o fprocess.o just.o

rpl_LIBS = -lutil -lobj -lc
just_LIBS = -lutil -lc

LIBFLAGS = -L${MAKETOP}/tenplus/lib/util -L${MAKETOP}/tenplus/lib/obj

fjust_IDIR = /usr/bin/
just_IDIR = /usr/bin/
rpl_IDIR = /usr/bin/
fformat_IDIR = /usr/bin/

ILIST = fjust just rpl fformat

.include <${RULES_MK}>
