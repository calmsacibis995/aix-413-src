# @(#)09        1.3  src/bos/usr/sbin/lvm/intercmd/Makefile, cmdlvm, bos41J, 9507A 2/2/95 17:02:59
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

VPATH		= ..

SCRIPTS		= redefinevg synclvodm updatelv updatevg

PROGRAMS	= getlvodm putlvodm getlvname getvgname lvchkmajor \
		  lvlstmajor lvgenmajor lvgenminor lvrelmajor lvrelminor getlvcb \
		  putlvcb migfix copyrawlv allocp lvmmsg lmktemp \
		  ipl_varyon tstresp
MSGHDRS		= cmdlvm_msg.h
CATFILES	= 
HFILES		= ${MSGHDRS}

ILIST		= ${PROGRAMS} ${SCRIPTS}
IDIR		= /usr/sbin/

INCFLAGS	= -I.. -I../include

allocp_OFILES	= allocp.o mapread.o place1copy.o pvbypv.o best_alloc.o \
		  map_alloc.o getnewpps.o deletemap.o max_alloc.o placelps.o

LIBFLAGS	= -L../liblv

LIBS		= -lsm -lcfg -llvm -lodm -lrts

.include <${RULES_MK}>
