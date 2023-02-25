# @(#)41        1.7  src/packages/bos/rte/etc/security/Makefile, pkgbos, pkg41J, 9520A_c 5/18/95 16:26:07
#
# COMPONENT_NAME: pkgbos
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		=${MAKETOP}/packages/bos/rte

OTHERS		=sysck.cfg	

.include <${RULES_MK}>

sysck.cfg: bos.rte.il ${MAKETOP}packages/bos/rte/shipfile_dependency
	@-${RM} -f sysck.cfg
	${TOUCH} sysck.cfg

.include <${MAKETOP}/packages/bos/rte/subsys.mk>
