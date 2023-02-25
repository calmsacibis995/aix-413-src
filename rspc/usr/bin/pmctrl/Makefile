# @(#)92	1.6  src/rspc/usr/bin/pmctrl/Makefile, pwrcmd, rspc41J, 9516B_all 4/21/95 15:21:32
# COMPONENT_NAME: (PWRCMD) Power Management
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

#CFLAGS		= -DPM_DEBUG

PROGRAMS	= pmctrl
MSGHDRS		= pmctrl_msg.h
CATFILES	= pmctrl.cat
HFILES		= ${MSGHDRS}

OTHERS		= pmmi_data

OFILES		= pmctrl.o pm_data.o pmodm.o get_pmodm.o
OPT_LEVEL	=

ILIST		= ${PROGRAMS} ${OTHERS}
IDIR		= /usr/bin/
pmmi_data_IDIR	= /etc/objrepos/

pmctrl_IMPORTS	= -bI:../../../kernext/pm/pmmi/pmmi.exp

LIBS		= -lodm -lcfg -lpm

.include <${RULES_MK}>

pmctrl.o: pmctrl_msg_default.h
get_pmodm.o: pmodm.h

pmmi_data: pmodm.crex pmmi_data.add
	${RM} -f pmmi_data pmmd_start
	ODMDIR=. ${ODMCREATE} -p pmodm.crex
	ODMDIR=. ${ODMADD} ${pmmi_data.add:P} \
> /dev/null || { ${RM} -f pmmi_data pmmd_data; false; }

pmodm.h: pmodm.crex
	${RM} -f pmodm.h
	ODMDIR=. ${ODMCREATE} -ph pmodm.crex

pmodm.crex: pmodm.cre
	${CP} ${pmodm.cre:P} ${.TARGET}

pmctrl_msg_default.h:	pmctrl.msg
	${SED} -e 's/^\$$.*$$//' < ${pmctrl.msg:P} > temp1
	${SED} -e '/^[ 	]*$$/d' temp1 > temp2
	${SED} -e 's/^\([^ 	\\]\)\(.*\)/#define DFLT_\1\2/' temp2 > temp3
	${SED} -e 's/"[^:]*: *[0-9]\{4,4\}-[0-9]\{3,3\} */"/' temp3 \
> pmctrl_msg_default.h
	${RM} -f temp1
	${RM} -f temp2
	${RM} -f temp3



