# @(#)24        1.13  src/packages/bos/rte/devices/Makefile, pkgbossub, pkg41J, 9523A_all 6/5/95 16:08:42
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
VPATH		= ..:root:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

OPTIONS		= bos.rte.devices

ROOT_OPTIONS	= bos.rte.devices

FILESET_BASE	= devices
SCRIPTS		= bos.rte.devices.add
OTHERS		= swvpd.make swvpdroot.make

.if (${PTF_UPDATE} == "yes")
USR_ODMADD_LIST		+= devno.add%bos.rte.devices
.else
PRELOADED_ODMADD_LIST	+= devno.add%bos.rte.devices
.endif

.include <../subsys.mk>
.include <packdep.mk>
.include <${RULES_MK}>
