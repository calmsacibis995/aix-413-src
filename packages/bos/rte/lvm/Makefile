# @(#)76        1.11  src/packages/bos/rte/lvm/Makefile, pkgbossub, pkg41J, 9520A_c 5/18/95 16:26:13
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
VPATH		= ..:${MAKETOP}/bos/objclass

RTE_TCB		= true

OPTIONS		= bos.rte.lvm

ROOT_OPTIONS	= bos.rte.lvm

FILESET_BASE	= lvm
SCRIPTS		= bos.rte.lvm.add
OTHERS		= swvpd.make sysck.make swvpdroot2.make

USR_ODMADD_LIST += cfgrule.lvm.add%bos.rte.lvm \
		   cmdps.add%bos.rte.lvm \
		   logvol.add%bos.rte.lvm \
		   sm_lvm.add%bos.rte.lvm

ROOT_ODMADD_LIST += lvm.add%bos.rte.lvm

swvpdroot2.make : bos.rte.${FILESET_BASE}.add INVENTORY_LIST $${ROOTOBJREPOS} \
		$${BIG_SFDEP}
	ODMDIR=${MAKETOP}packages/bos/rte/etc/objrepos ${ODMADD} ${bos.rte.${FILESET_BASE}.add:P}
	${TOUCH} swvpdroot2.make

.include <../subsys.mk>
.include <${RULES_MK}>
