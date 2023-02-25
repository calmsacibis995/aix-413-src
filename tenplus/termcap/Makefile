# @(#)33        1.24  src/tenplus/termcap/Makefile, tenplus, tenplus41J, 9523C_all 6/7/95 13:55:43
#
# COMPONENT_NAME: (TENPLUS) INed Editor
#
# ORIGINS:  9, 10, 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1985, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# trm files define the input and output sequences for terminals.
# map files are keyboard maps intended to be read by people.
# trm files are created from tsz (terminal stanza) files.
# map files are created from msz (map stanza) files.

VPATH = ${MAKETOP}/tenplus/e2/common:${MAKETOP}/tenplus/util
PROGRAMS = tdigest
tdigest_OFILES = tdigest.o inseqs.o
OTHERS = keymaps def.trm keys.map terms.bin inseq.frm ${OFMS}
MSGHDRS = Hkeys.map_msg.h INeditor_msg.h
HFILES = ${MSGHDRS}
CATFILES = Hkeys.map.cat INeditor.cat
LIBS = -lprf -lutil -lobj 
LIBFLAGS = -L${MAKETOP}/tenplus/lib/prf -L${MAKETOP}/tenplus/lib/util -L${MAKETOP}/tenplus/lib/obj
SCRIPTS = keymaps.sh
INCFLAGS = -I${MAKETOP}/tenplus/include -I${MAKETOP}/tenplus/keys -I${MAKETOP}/tenplus/util

keymaps_IDIR = /usr/bin/
def.trm_IDIR = /usr/lib/INed/
keys.map_IDIR = /usr/lib/INed/
terms.bin_IDIR = /usr/lib/INed/
inseq.ofm_IDIR = /usr/lib/INed/forms/
trm.ofm_IDIR = /usr/lib/INed/forms/
map.ofm_IDIR = /usr/lib/INed/forms/
mappict.ofm_IDIR = /usr/lib/INed/forms/

ILIST = keymaps def.trm keys.map terms.bin inseq.ofm  \
        trm.ofm map.ofm mappict.ofm


BOTHTYPES = default.both lft.both dtterm.both \
            ibm3101.both ibm3161.both ibm3151.both ibm3151-51.both \
	    ibm5550.both ibm5570.both vt100.both vt220.both \
            vt100x.both wyse.both wyse30.both xterm.both

MAPTYPES = ${BOTHTYPES:.both=.map} vi640.map 

TERMTYPES = ${BOTHTYPES:.both=.trm}

# def.trm is a compendium of terminal information about lots of terminal
# types.
def.trm: ${TERMTYPES}
	 ${SCAT}  ${TERMTYPES} def.trm

# terms.bin is a compiled version of def.trm.
terms.bin: def.trm
	 ${TDIGEST} ${def.trm:P} terms.bin

# keys.map is a compendium of keymap files.
keys.map: ${MAPTYPES}
	 ${SCAT}  ${MAPTYPES} keys.map

# Form files and compiled form files
FRMS     = inseq.frm trm.frm map.frm mappict.frm
OFMS     = ${FRMS:.frm=.ofm}

.include <${RULES_MK}>
