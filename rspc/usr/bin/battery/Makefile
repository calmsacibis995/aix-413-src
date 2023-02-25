# @(#)89	1.4  src/rspc/usr/bin/battery/Makefile, pwrcmd, rspc41J, 9513A_all 3/28/95 16:21:12
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

PROGRAMS	= battery
MSGHDRS		= battery_msg.h
CATFILES	= battery.cat
HFILES		= ${MSGHDRS}
#CFLAGS 	= -DPM_DEBUG

ILIST		= battery
IDIR		= /usr/bin/

battery_IMPORTS	= -bI:../../../kernext/pm/pmmd/pmmd.exp

LIBS        = -lpm

.include <${RULES_MK}>

battery.o: battery_msg_default.h

battery_msg_default.h:	battery.msg
	${SED} -e 's/^\$$.*$$//' < ${battery.msg:P} > temp1
	${SED} -e '/^[ 	]*$$/d' temp1 > temp2
	${SED} -e 's/^\([^ 	\\]\)\(.*\)/#define DFLT_\1\2/' temp2 > temp3
	${SED} -e 's/"[^:]*: *[0-9]\{4,4\}-[0-9]\{3,3\} */"/' temp3 \
> battery_msg_default.h
	${RM} -f temp1
	${RM} -f temp2
	${RM} -f temp3
