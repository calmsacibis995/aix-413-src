# @(#)76	1.11  src/tenplus/helpers/history/Makefile, tenplus, tenplus411, GOLD410 5/5/94 12:51:01
#
# COMPONENT_NAME: (TENPLUS) INed Editor
#
# FUNCTIONS:
#
# ORIGINS:  9, 10, 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1985, 1994
# All Rights Reserved
# 
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

VPATH = ${MAKETOP}/tenplus/e2/common
PROGRAMS = history.help
history.help_OFILES = history.o u9.o stdalone.o 
MSGHDRS = INhistory_msg.h INeditor_msg.h
HFILES = ${MSGHDRS}
CATFILES = INeditor.cat INhistory.cat
OTHERS = hist.frm hist.ofm
LIBS = -lhelp -lobj
LIBFLAGS = -L${MAKETOP}/tenplus/hlib/help -L${MAKETOP}/tenplus/lib/obj

hist.ofm_IDIR = /usr/lib/INed/forms/
history.help_IDIR = /usr/lib/INed/helpers/

ILIST = hist.ofm history.help
INCFLAGS = -I${MAKETOP}/tenplus/include -I${MAKETOP}/tenplus/keys  

.include <${RULES_MK}>

hist.frm: hist.fsz INeditor_msg.h
	${TOSF} ${hist.fsz:P} INeditor_msg.h
hist.ofm : hist.frm
	${FC} ${hist.frm:P}
