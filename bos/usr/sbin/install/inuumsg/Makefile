# @(#)25        1.5  src/bos/usr/sbin/install/inuumsg/Makefile, cmdinstl, bos41J, 9521A_all 5/22/95 08:40:57
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

PROGRAMS	= inuumsg
MSGHDRS		= inuumsg_msg.h
CATFILES	= inuumsg.cat
HFILES		= ${MSGHDRS} def_msg.h

ILIST		= inuumsg
IDIR		= /usr/sbin/


.include <${RULES_MK}>

def_msg.h:	inuumsg.msg
	${RM} -f ${.TARGET} inuumsg.msg.X
	${CP} ${.ALLSRC} ${.ALLSRC:T}.X
	${ECHO} "1,\$$g/^\\\$$/d\n1,\$$s/INU_[^ ]* /\n1,\$$s/\"\$$/\",/\n\$$s/\",\$$/\"};/\n0a\nchar *def_msg[] = {\n\"index 0\",\n.\nw def_msg.h\nq" | ${ED} inuumsg.msg.X

