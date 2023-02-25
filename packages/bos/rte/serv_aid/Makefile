# @(#)02        1.8  src/packages/bos/rte/serv_aid/Makefile, pkgbossub, pkg411, GOLD410 6/22/94 11:53:42
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

OPTIONS		= bos.rte.serv_aid

ROOT_OPTIONS	= bos.rte.serv_aid

FILESET_BASE	= serv_aid
SCRIPTS		= bos.rte.serv_aid.add
OTHERS		= swvpd.make swvpdroot.make

USR_ODMADD_LIST += base_dump.add%bos.rte.serv_aid \
		   base_errlg.add%bos.rte.serv_aid \
		   sm_alog.add%bos.rte.serv_aid

.include <../subsys.mk>
.include <${RULES_MK}>
