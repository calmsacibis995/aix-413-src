# @(#)50	1.5  src/rspc/kernext/pci/ncr810/Makefile, pciscsi, rspc41J, 9507A 1/25/95 13:32:09
#
#   COMPONENT_NAME: PCISCSI
#
#   FUNCTIONS: defined
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# Makefile for the NCR 53C8xx device driver
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

KERNEL_EXT	= ncr810dd
IDIR		= /usr/lib/drivers/pci/
ILIST		= ncr810dd

ncr810dd_ENTRYPOINT	= p8xx_config

LIBS		= ${KERNEXT_LIBS}

ncr810dd_OFILES = ncr8xxt.o ncr8xxb.o ncr8xxu.o ncr8xx_getfree.o

.include <${RULES_MK}>
