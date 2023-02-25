# @(#)97        1.8  src/packages/bos/rte/jfscomp/Makefile, pkgbossub, pkg41J, 9520A_c 5/18/95 16:26:11
#
# COMPONENT_NAME: pkgbossub
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
VPATH		= ..:root

OPTIONS		= bos.rte.jfscomp

ROOT_OPTIONS	= bos.rte.jfscomp

FILESET_BASE	= jfscomp
SCRIPTS		= bos.rte.jfscomp.add
OTHERS		= swvpd.make  swvpdroot.make

# Add vpd information in the swvpd databases
swvpd.make : bos.rte.jfscomp.add INVENTORY_LIST $${USROBJREPOS} $${BIG_SFDEP}
	ODMDIR=${MAKETOP}packages/bos/rte/usr/lib/objrepos ${ODMADD} ${bos.rte.jfscomp.add:P}
	${TOUCH} swvpd.make

.include <../subsys.mk>
.include <${RULES_MK}>
