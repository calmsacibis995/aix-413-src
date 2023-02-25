# @(#)81	1.8  src/bos/usr/sbin/adfutil/Makefile, cmdadf, bos41J, 9521B_all 5/24/95 11:50:37
#
#   COMPONENT_NAME: CMDADF
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1989,1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

DOSPATH = ${MAKETOP}/bos/usr/bin/dosdir
VPATH = ${DOSPATH}
INCFLAGS = -I${DOSPATH}

PROGRAMS  = adfutil
MSGHDRS   = adf_msg.h
CATFILES        = adf.cat
HFILES   = ${MSGHDRS} pcdos.h tables.h

ILIST   = adfutil
IDIR   =  /usr/sbin/

DOSOBJ = DFstr.o analyze.o dchdir.o dclose.o dcreate.o dfirst.o dlock.o \
	dlseek.o dmount.o dnext.o dopen.o doschdir.o dosinit.o dospwd.o \
	dread.o dremove.o dunmount.o dwrite.o getnewcl.o locate.o makedate.o \
	pause.o putfat.o reclaim.o

OFILES = y.tab.o lex.yy.o dossup.o file.o funct.o main.o make.o micro.o gen.o \
	dosclose.o dosfirst.o dosnext.o dosopen.o dosread.o dosseek.o \
	doswrite.o util.o devio.o semafore.o dfstat.o makeopen.o \
	${DOSOBJ}

LIBS   = -ll -lodm -lcfg  -lIN

.include <${RULES_MK}>

y.tab.o_CFLAGS = -DYACC_MSG

tn.h: parser.y y.tab.c
		@${RM} -f ${.TARGET} y.tab.c y.tab.h
		${YACC} -d ${parser.y:P}
		${MV} y.tab.h tn.h

y.tab.c:
		@${RM} -f ${.TARGET}
		${TOUCH} ${.TARGET}

lex.yy.c: scanner.l
		${LEX} ${scanner.l:P}

y.tab.o:   symbol.h externs.h tn.h y.tab.c

lex.yy.o:  symbol.h tn.h externs.h

${OBJECTS} : adf_msg.h

