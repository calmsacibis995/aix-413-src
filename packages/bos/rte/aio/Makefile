# @(#)91        1.12  src/packages/bos/rte/aio/Makefile, pkgbossub, pkg41J, 9520A_c 5/18/95 16:26:02
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

OPTIONS		= bos.rte.aio

ROOT_OPTIONS	= bos.rte.aio

FILESET_BASE	= aio
SCRIPTS		= bos.rte.aio.add
OTHERS		= swvpd.make swvpdroot2.make

USR_ODMADD_LIST += aio.add%bos.rte.aio \
		   sm_aio.add%bos.rte.aio

ROOT_ODMADD_LIST += cfgrule.aio.add%bos.rte.aio

swvpdroot2.make : bos.rte.${FILESET_BASE}.add INVENTORY_LIST $${ROOTOBJREPOS}  \
		$${BIG_SFDEP}
	ODMDIR=${MAKETOP}packages/bos/rte/etc/objrepos ${ODMADD} ${bos.rte.${FILESET_BASE}.add:P}
	${TOUCH} swvpdroot2.make

.include <../subsys.mk>
.include <${RULES_MK}>
