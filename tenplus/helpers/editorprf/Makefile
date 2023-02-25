# @(#)05	1.15  src/tenplus/helpers/editorprf/Makefile, tenplus, tenplus411, 9434B411a 8/23/94 14:14:27
#
# COMPONENT_NAME: TENPLUS
#
# FUNCTIONS: Makefile
#
# ORIGINS:  9, 10, 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH = ${MAKETOP}/tenplus/e2/common
PROGRAMS = edprf.help
OTHERS = editorprf.help editorprf edprf.hdq ${FRMS} ${OFMS}

edprf.help_OFILES = editorprf.o stdalone.o 

CATFILES = INeditorprf.cat INeditor.cat Hedprf.hdq.cat
MSGHDRS = INeditorprf_msg.h INeditor_msg.h
HFILES = ${MSGHDRS}
INCFLAGS = -I${MAKETOP}/tenplus/include -I${MAKETOP}/tenplus/keys

LIBS = -lhelp -lutil -lobj -lc
LIBFLAGS = -L${MAKETOP}/tenplus/hlib/help -L${MAKETOP}/tenplus/lib/util -L${MAKETOP}/tenplus/lib/obj

editorprf.help_IDIR = /usr/lib/INed/helpers/
editorprf_IDIR = /usr/lib/INed/
edprf.hdq_IDIR = /usr/lib/INed/
editorprf.ofm_IDIR = /usr/lib/INed/forms/
pmenu.ofm_IDIR = /usr/lib/INed/forms/
phelp.ofm_IDIR = /usr/lib/INed/forms/
pwatch.ofm_IDIR = /usr/lib/INed/forms/
ppaths.ofm_IDIR = /usr/lib/INed/forms/
pother.ofm_IDIR = /usr/lib/INed/forms/
etask.ofm_IDIR = /usr/lib/INed/forms/
hdq.ofm_IDIR = /usr/lib/INed/forms/

ILIST = editorprf.help editorprf edprf.hdq editorprf.ofm pmenu.ofm \
        phelp.ofm pwatch.ofm ppaths.ofm pother.ofm etask.ofm hdq.ofm


.include <${RULES_MK}>

# explicit rules for form-files with long names
editorprf.frm: edprf.fsz INeditor_msg.h
	${TOSF} ${edprf.fsz:P} INeditor_msg.h

helpmenu: helpmenu.psz INeditor_msg.h
	${MRI2SF} ${helpmenu.psz:P} INeditor_msg.h

menu: menu.psz INeditor_msg.h
	${MRI2SF} ${menu.psz:P} INeditor_msg.h

other: other.psz INeditor_msg.h
	${MRI2SF} ${other.psz:P} INeditor_msg.h

paths: paths.psz INeditor_msg.h
	${MRI2SF} ${paths.psz:P} INeditor_msg.h

watchfiles: watchfil.psz INeditor_msg.h
	${MRI2SF} ${watchfil.psz:P} INeditor_msg.h

edprf.hdq.psz: edprf.awk edprf.epii
	${AWK} -f ${edprf.awk:P} ${edprf.epii:P} > ${.TARGET}

Hedprf.hdq.msg: edprf.msg.awk edprf.epii
	${AWK} -f ${edprf.msg.awk:P} ${edprf.epii:P} > ${.TARGET}

edprf.hdq: edprf.hdq.psz
	${MRI2SF} edprf.hdq.psz

PRFS    = menu helpmenu watchfiles paths other
editorprf: ${PRFS}
	${SCAT} ${PRFS} editorprf

editorprf.help: edprf.help
	${CP} ${edprf.help:P} ${.TARGET}

FRMS    = editorprf.frm pmenu.frm phelp.frm pwatch.frm \
          ppaths.frm  pother.frm  etask.frm hdq.frm
OFMS    = $(FRMS:.frm=.ofm)
