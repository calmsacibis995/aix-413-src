# @(#)78	1.11  src/tenplus/helpers/dir/Makefile, tenplus, tenplus411, GOLD410 6/8/93 13:50:44
#
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


VPATH = ${MAKETOP}/tenplus/e2/common
PROGRAMS = index.help 

MSGHDRS = INindex_msg.h INeditor_msg.h
OTHERS = indexprf ${FRMS} ${OFMS} 
HFILES = ${MSGHDRS}
CATFILES = INindex.cat INeditor.cat
LIBS =	-lprf -lhelp -lobj -lDT -lutil -ls 
LIBFLAGS = -L${MAKETOP}/tenplus/lib/prf -L${MAKETOP}/tenplus/hlib/help -L${MAKETOP}/tenplus/lib/obj -L${MAKETOP}/tenplus/hlib/DT -L${MAKETOP}/tenplus/lib/util 

OFILES = externs.o buttons.o files.o ls.o stdalone.o statmod.o \
         uinit.o umod.o
index.help_OFILES = ${OFILES}

INCFLAGS = -I${MAKETOP}/tenplus/include -I${MAKETOP}/tenplus/e2/include -I${MAKETOP}/tenplus/keys 

index.help_IDIR = /usr/lib/INed/helpers/
index.ofm_IDIR = /usr/lib/INed/forms/
tindex.ofm_IDIR = /usr/lib/INed/forms/
ls.ofm_IDIR = /usr/lib/INed/forms/
tls.ofm_IDIR = /usr/lib/INed/forms/
stat.ofm_IDIR = /usr/lib/INed/forms/
tstat.ofm_IDIR = /usr/lib/INed/forms/
indexprf.ofm_IDIR = /usr/lib/INed/forms/
indexprf_IDIR = /usr/lib/INed/

ILIST = index.help index.ofm tindex.ofm ls.ofm tls.ofm stat.ofm \
        tstat.ofm indexprf.ofm indexprf

.include <${RULES_MK}>

# form files, compiled form files
FRMS    = index.frm tindex.frm ls.frm tls.frm stat.frm \
          tstat.frm indexprf.frm
OFMS    = $(FRMS:.frm=.ofm)

indexprf : indexprf.psz INeditor_msg.h
	${MRI2SF} ${indexprf.psz:P} ${INeditor_msg.h:P}
