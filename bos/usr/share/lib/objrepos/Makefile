# @(#)16        1.4  src/bos/usr/share/lib/objrepos/Makefile, Makefiles, bos411, 9428A410j 4/1/94 08:47:08
#
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/usr/lib/objrepos/swvpd

OTHERS		= lpp lpp.vc product product.vc history history.vc \
		  inventory inventory.vc

ILIST		= ${OTHERS}
IDIR		= /usr/share/lib/objrepos/

NOUPDATE_LIST   = /usr/share/lib/objrepos/lpp \
		  /usr/share/lib/objrepos/lpp.vc \
		  /usr/share/lib/objrepos/product \
		  /usr/share/lib/objrepos/product.vc \
		  /usr/share/lib/objrepos/history \
		  /usr/share/lib/objrepos/history.vc \
		  /usr/share/lib/objrepos/inventory \
		  /usr/share/lib/objrepos/inventory.vc

.include <${RULES_MK}>

${OTHERS} :	swvpd.make

swvpd.make :	swvpd.cre
	${RM} -f ${OTHERS} 
	${ECHO} creating swvpd
	${ODMCREATE} -c ${swvpd.cre:P}
	${TOUCH} swvpd.make
