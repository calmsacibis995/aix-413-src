# @(#)04	1.16  src/tenplus/helpers/print/Makefile, tenplus, tenplus411, GOLD410 3/4/94 17:10:15
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

# form files and compiled form files

VPATH = ${MAKETOP}/tenplus/e2/common
PROGRAMS = print.help prtty
OTHERS = ${FRMS} ${OFMS} printprf prthelp
prtty_OFILES = prtty.o
print.help_OFILES = print.o fixcmd.o

MSGHDRS = Hprthelp_msg.h prtty_msg.h INprint_msg.h INeditor_msg.h
HFILES = ${MSGHDRS}
CATFILES = Hprthelp.cat prtty.cat INprint.cat INeditor.cat

print.help_LIBS = -lwin -lhelp -lutil -lprf -lshort -lobj -lcurses
prtty_LIBS = -lutil -lobj -lcurses
LIBFLAGS = -L${MAKETOP}/tenplus/lib/window -L${MAKETOP}/tenplus/hlib/help -L${MAKETOP}/tenplus/lib/util -L${MAKETOP}/tenplus/lib/prf -L${MAKETOP}/tenplus/lib/short -L${MAKETOP}/tenplus/lib/obj ${_AIX3CURSESLIBFLAG}


CFLAGS	+= -DVBLSCR ${_AIX3CURSESFLAGS}

print.help_IDIR = /usr/lib/INed/helpers/
prtty_IDIR  = /usr/bin/
printprf_IDIR = /usr/lib/INed/
printprf.ofm_IDIR = /usr/lib/INed/forms/
prtprfz.ofm_IDIR = /usr/lib/INed/forms/
prthelp.ofm_IDIR = /usr/lib/INed/forms/
prthelpz.ofm_IDIR = /usr/lib/INed/forms/
prthelp_IDIR  = /usr/lib/INed/

ILIST = print.help prtty printprf printprf.ofm prtprfz.ofm prthelp.ofm \
        prthelpz.ofm prthelp

INCFLAGS = -I${MAKETOP}/tenplus/include -I${MAKETOP}/tenplus/keys 


.include <${RULES_MK}>


prthelp: prthelp.psz Hprthelp_msg.h
	${MRI2SF} ${prthelp.psz:P} ${Hprthelp_msg.h:P}

printprf: printprf.psz INeditor_msg.h
	${MRI2SF} ${printprf.psz:P} ${INeditor_msg.h:P}

FRMS    = printprf.frm prtprfz.frm prthelp.frm prthelpz.frm
OFMS    = $(FRMS:.frm=.ofm)
