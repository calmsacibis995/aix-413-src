# @(#)54	1.15  src/packages/devices/msg/en_US/Makefile, pkgmsg, pkg411, 9436C411a 9/7/94 05:17:04
#
# COMPONENT_NAME: pkgmsg
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993,1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
ENG		= ${MAKETOP}/packages/devices/msg/en_US

VPATH		= ${ENG}/diag/rte:${ENG}/base/com:${ENG}/sys/mca/rte:${ENG}/rspc/base/com

BFF		= devices.msg.${MSGLANG}

ILIST		= ${BFF}
IDIR		= /

OPTIONS 	= devices.msg.${MSGLANG}.base.com \
		  devices.msg.${MSGLANG}.diag.rte \
		  devices.msg.${MSGLANG}.sys.mca.rte \
		  devices.msg.${MSGLANG}.rspc.base.com

diag.rte_INSTREQ = bos.diag.rte

.include <${MAKETOP}/packages/bos/msg/instreq.mk>
