# @(#)99        1.9  src/packages/bos/rte/security/Makefile, pkgbossub, pkg41J, 9523A_all 6/5/95 16:09:28
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
VPATH		= ..:root:${MAKETOP}/bos/objclass

RTE_TCB		= true

OPTIONS		= bos.rte.security

ROOT_OPTIONS	= bos.rte.security

FILESET_BASE	= security
SCRIPTS		= bos.rte.security.add
OTHERS		= swvpd.make swvpdroot.make sysck.make sysckroot.make

USR_ODMADD_LIST += usrgrp.add%bos.rte.security

.include <../subsys.mk>
.include <packdep.mk>
.include <${RULES_MK}>
