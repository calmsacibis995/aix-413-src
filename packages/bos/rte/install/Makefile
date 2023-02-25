# @(#)45        1.10  src/packages/bos/rte/install/Makefile, pkgbossub, pkg41J, 9523C_all 6/8/95 16:44:40
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
# (C) COPYRIGHT International Business Machines Corp. 1994, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
VPATH		= ..:${MAKETOP}/bos/objclass:usr

RTE_TCB		= true

OPTIONS		= bos.rte.install

FILESET_BASE	= install
SCRIPTS		= bos.rte.install.add
OTHERS		= swvpd.make sysck.make

USR_ODMADD_LIST += tcb.add%bos.rte.install \
		   swvpd.add%bos.rte.install

USR_LIBLPP_UPDT_LIST += bos.rte.install.post_u \
			bos.rte.install.unpost_u

.include <../subsys.mk>
.include <${RULES_MK}>
