# @(#)16	1.20  src/tenplus/e2/common/Makefile, tenplus, tenplus41J, 9517A_all 4/24/95 21:10:56
#
# COMPONENT_NAME: (INED) INed Editor
# 
# ORIGINS:  9, 10, 27
# 
# This module contains IBM CONFIDENTIAL code. -- (IBM
# Confidential Restricted when combined with the aggregated
# modules for this product)                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1985, 1995
# All Rights Reserved
# 
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 

VPATH = ${MAKETOP}/tenplus/interface:${MAKETOP}/tenplus/keys
PROGRAMS	= poundfile te
OTHERS = e.cmds e.howtos emenu ${FRMS} ${OFMS} 
SCRIPTS = cleandir housekeep

# BHELPO macro should only be set if helpers are being built in
BHELPO     = bltinhelp.o
PIPEO      = pipe.o

tables.c: ed.keys
	${RM} -f tables.c keynames.h names.h readkeys
	${KEYS} ${ed.keys:P}

ed.keys: keys.stz
	${TOSF} ${keys.stz:P}


OBJECTS     =  $(BHELPO) \
    cmds.o               \
    cursor.o             \
    disp1.o              \
    disp2.o              \
    ed.o                 \
    eimports.o           \
    fake.o               \
    form.o               \
    format.o             \
    globals.o            \
    gng.o                \
    hep.o                \
    imports.o            \
    in.o                 \
    info.o               \
    intran.o             \
    menus.o              \
    misc.o               \
    mn.o                 \
    nfl.o                \
    out.o                \
    $(PIPEO)             \
    pop.o                \
    put.o                \
    salloc.o             \
    sp.o                 \
    src.o                \
    stubs.o              \
    tables.o             \
    text.o               \
    tv.o                 \
    win.o                \
    window.o             \
    wrp.o                \
    zoom.o                

te_OFILES = ${OBJECTS}

MSGHDRS = poundfile_msg.h INeditor_msg.h He.cmds_msg.h He.howtos_msg.h \
					Hemenu_msg.h

HFILES = ${MSGHDRS}
CATFILES = poundfile.cat INeditor.cat He.cmds.cat He.howtos.cat Hemenu.cat

LIBS = -lwin -lshort -lobj -lutil -ls -lcurses
LIBFLAGS = -L${MAKETOP}/tenplus/lib/short -L${MAKETOP}/tenplus/lib/obj -L${MAKETOP}/tenplus/lib/util -L${MAKETOP}/tenplus/hlib/help -L${MAKETOP}/tenplus/lib/window ${_AIX3CURSESLIBFLAG}

e.cmds: ecmds.psz He.cmds_msg.h
	${MRI2SF} ${ecmds.psz:P} ${He.cmds_msg.h:P}

e.howtos: ehowtos.psz He.howtos_msg.h
	${MRI2SF} ${ehowtos.psz:P} ${He.howtos_msg.h:P}

emenu: emenu.psz Hemenu_msg.h
	${MRI2SF} ${emenu.psz:P} ${Hemenu_msg.h:P}


te_IDIR = /usr/bin/
poundfile_IDIR = /usr/lib/INed/bin/
std.ofm_IDIR = /usr/lib/INed/forms/
sindex.ofm_IDIR = /usr/lib/INed/forms/
howto.ofm_IDIR = /usr/lib/INed/forms/
howtos.ofm_IDIR = /usr/lib/INed/forms/
cmd.ofm_IDIR = /usr/lib/INed/forms/
cmds.ofm_IDIR = /usr/lib/INed/forms/
emenu.ofm_IDIR = /usr/lib/INed/forms/
emenuz.ofm_IDIR = /usr/lib/INed/forms/
e.cmds_IDIR = /usr/lib/INed/
e.howtos_IDIR = /usr/lib/INed/
emenu_IDIR = /usr/lib/INed/
housekeep_IDIR = /usr/bin/
cleandir_IDIR = /usr/bin/

ILIST = te poundfile std.ofm sindex.ofm howto.ofm howtos.ofm cmd.ofm cmds.ofm \
        emenu.ofm emenuz.ofm e.cmds e.howtos emenu housekeep cleandir

CFLAGS += -DBLTHELP -DPIPEHELP -DVBLSCR -DPIPETRACE ${_AIX3CURSESFLAGS}

INCFLAGS = -I${MAKETOP}/tenplus/include -I${MAKETOP}/tenplus/e2/include -I${MAKETOP}/tenplus/interface -I${MAKETOP}/tenplus/keys

.include <${RULES_MK}>

FRMS    = std.frm sindex.frm howto.frm howtos.frm cmd.frm cmds.frm emenu.frm \
          emenuz.frm
OFMS    = $(FRMS:.frm=.ofm)

