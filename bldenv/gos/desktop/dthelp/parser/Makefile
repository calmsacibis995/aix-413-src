# @(#)78        1.4  src/bldenv/gos/desktop/dthelp/parser/Makefile, desktop, bos412, GOLDA411a 4/25/94 14:19:29
#
#   COMPONENT_NAME: DESKTOP
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

HTAG 		= ${MAKETOP}/gos/desktop/cde1/dthelp/parser/htag

VPATH 		= ${HTAG}/parser:${HTAG}/helptag:${HTAG}/eltdef

SUBDIRS		= helptag pass1 pass2

PROGRAMS 	= parser
OTHERS		= htag
HFILES		= includes/context.h includes/if.h

OFILES 		= parser.o actutil.o closent.o code.o cyclent.o dcurelt.o \
		  end.o entity.o error.o getsign.o input.o level.o lookent.o \
		  minim.o optstrg.o param.o parent.o parutil.o pi.o \
		  scan.o setpar.o start.o string.o struct.o text.o \
		  custom.o default.o help.o make.o option.o out.o xref.o

CFLAGS 		= -DSYSV -DAIXV3 -D_ALL_SOURCE -D_AIX -D__AIX -D__aix -D_aix \
		  -Daix -DMULTIBYTE -DNLS16 -DMESSAGE_CAT -DAIX_ILS -DR5TREE \
		  -DVALUE_ADDED -qMAXMEM=50000

INCFLAGS 	= -Iincludes -I${HTAG}/parser -I${HTAG}/util -I${HTAG}/helptag \
		  -I${HTAG}/build -I../eltdef

LIBS 		=  -L../util -lutil

ILIST 		= parser htag
IDIR 		= /opt/dt/etc/

.include <${RULES_MK}>

includes/context.h includes/if.h: delim.par context.dat help.if
	${RM} -rf ${.TARGET:H}
	${MKDIR} ${.TARGET:H}
	${CP} ${delim.par:P} ${.TARGET:H}/delim.dat
	${CP} ${context.dat:P} ${.TARGET:H}
	cd ${.TARGET:H}; ${DTCONTEXT} 
	${CP} ${help.if:P} ${.TARGET:H}
	cd ${.TARGET:H}; ${DTELTDEF} help.if

htag:  parser
	${RM} -f htag
	${CP} ${parser:P} htag
