# @(#)23	1.20  src/packages/bos/msg/en_US/Makefile, pkgmsg, pkg411, GOLD410 4/12/94 08:36:41
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
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

ENG		= ${MAKETOP}/packages/bos/msg/en_US

VPATH		= ${ENG}/rte:${ENG}/rte/mp:${ENG}/diag/rte
VPATH		:= ${VPATH}:${ENG}/INed:${ENG}/net/tcp/client:${ENG}/txt/tfs

BFF		= bos.msg.${MSGLANG}

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= \
	bos.msg.${MSGLANG}.rte \
	bos.msg.${MSGLANG}.rte.mp \
	bos.msg.${MSGLANG}.diag.rte \
	bos.msg.${MSGLANG}.INed \
	bos.msg.${MSGLANG}.net.tcp.client \
	bos.msg.${MSGLANG}.txt.tfs

.include <${MAKETOP}/packages/bos/msg/instreq.mk>

