# @(#)95	1.8  src/rspc/usr/bin/pmd/Makefile, pwrcmd, rspc41J, 9520A_all 5/16/95 16:02:11
# COMPONENT_NAME: (PWRCMD) Power Management Command
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

CCTYPE 		= base_threaded
#CC_OPT_LEVEL 	= -g
#CFLAGS		= -DPM_DEBUG -DPM_DEBUG2
#CFLAGS		= -DPM_DEBUG 

VPATH		= ${MAKETOP}/rspc/usr/bin/pmctrl

PROGRAMS	= pmd 
MSGHDRS		= pmd_msg.h 
#CATFILES	= pmd.cat
HFILES		= ${MSGHDRS}
INCFLAGS	= -I${MAKETOP}/rspc/usr/ccs/lib/libpm -I${MAKETOP}/rspc/kernext/pm/pmmi

OTHERS		= pmd_msg_default.h

pmd_OFILES	= pmd_main.o pmd_core_event.o pmd_socket.o pmd_signal_waiter.o\
 pmodm.o get_pmodm.o odm_data.o firmware_support.o getpmlv.o 

ILIST		= pmd
IDIR		= /usr/bin/

pmd_IMPORTS	= -bI:../../../kernext/pm/pmmi/pmmi.exp  
 
LIBS		= -lodm -lcfg -ls -llvm -lsm -lpthreads


.include <${RULES_MK}>

get_pmodm.o: pmodm.h
odm_data.o: pmodm.h

pmodm.h: pmodm.crex
	${RM} -f pmodm.h
	ODMDIR=. ${ODMCREATE} -ph pmodm.crex

pmodm.crex: pmodm.cre
	${CP} ${pmodm.cre:P} ${.TARGET}

pmd_msg_default.h:	pmd.msg
	${SED} -e 's/^\$$.*$$//' < ${pmd.msg:P} > temp1
	${SED} -e '/^[ 	]*$$/d' temp1 > temp2
	${SED} -e 's/^\([^ 	\\]\)\(.*\)/#define DFLT_\1\2/' temp2 > temp3
	${SED} -e 's/"[^:]*: *[0-9]\{4,4\}-[0-9]\{3,3\} */"/' temp3 \
> pmd_msg_default.h
	${RM} -f temp1
	${RM} -f temp2
	${RM} -f temp3

