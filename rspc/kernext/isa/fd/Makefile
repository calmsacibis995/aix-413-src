# @(#)70        1.7  src/rspc/kernext/isa/fd/Makefile, isafd, rspc411, GOLD410 6/27/94 20:43:41
# COMPONENT_NAME:
#
# FUNCTIONS:
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

.include <${MAKETOP}bos/kernext/Kernext.mk>

KERNEL_EXT	= fdisa
IDIR		= /usr/lib/drivers/isa/
ILIST		= fdisa

fdisa_ENTRYPOINT	= fd_config

LIBS		= ${KERNEXT_LIBS}

fdisa_OFILES = fd.o fd_bot.o

.include <${RULES_MK}>
