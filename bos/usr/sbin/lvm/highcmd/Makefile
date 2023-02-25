# @(#)08        1.2  src/bos/usr/sbin/lvm/highcmd/Makefile, cmdlvm, bos411, 9428A410j 1/19/93 18:35:06
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

PROGRAMS	= varyonvg lslv lsvgfs lspv lsvg 
SCRIPTS		= chlv chpv chvg cplv extendlv extendvg exportvg \
		  importvg migratepv mklv mkvg reducevg reorgvg \
		  rmlv rmlvcopy syncvg varyoffvg cfgvg
MSGHDRS		= cmdlvm_msg.h
CATFILES	= 
HFILES		= ${MSGHDRS}

ILIST		= ${PROGRAMS} ${SCRIPTS}
IDIR		= /usr/sbin/

INCFLAGS	= -I.. -I../include

lslv_OFILES	= lslv.o parselslv.o
lspv_OFILES	= lspv.o parselspv.o
lsvg_OFILES	= lsvg.o parselsvg.o
varyonvg_OFILES	= varyonvg.o


LIBFLAGS	= -L../liblv

LIBS		= -lsm -llvm -lodm -lcfg

.include <${RULES_MK}>
