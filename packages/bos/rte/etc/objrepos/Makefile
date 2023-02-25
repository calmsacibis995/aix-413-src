# @(#)22        1.6  src/packages/bos/rte/etc/objrepos/Makefile, pkgbos, pkg41J, 9520A_c 5/18/95 16:26:05
#
# COMPONENT_NAME: pkgbos
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		 = ${MAKETOP}/bos/usr/lib/objrepos/swvpd
VPATH		:= ${VPATH}:${MAKETOP}/packages/bos/rte

SCRIPTS		= swvpd.add

OTHERS		= swvpd.root.make

.include <${RULES_MK}>

.include <../../subsys.mk>
swvpd.root.make : swvpd.cre swvpd.add bos.rte.il bos.rte.lp ${BOSRTE_ADDFILES} \
		${BIG_SFDEP}
	${RM} -f lpp lpp.vc product product.vc history history.vc \
		inventory inventory.vc
	${ODMCREATE} -c ${swvpd.cre:P}
	${ODMADD} ${swvpd.add:P}
	${TOUCH} ${.TARGET} 
