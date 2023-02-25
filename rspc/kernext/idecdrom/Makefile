# @(#)48        1.3  src/rspc/kernext/idecdrom/Makefile, idecd, rspc41J, 9510A 2/21/95 14:17:37
# 
# COMPONENT_NAME: IDECD
#
# FUNCTIONS: None
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

OTHERS                  = root/devices.ide.cdrom.rte.trc.S

LOCALCFLAGS		= -qmaxmem=23000 -DPM_SUPPORT

KERNEL_EXT		= idecdrom idecdrompin
IDIR			= /usr/lib/drivers/
ILIST			= idecdrom idecdrompin

LIBS			= ${KERNEXT_LIBS}

idecdrom_ENTRYPOINT	= idecdrom_config

idecdrom_IMPORTS	= -bI:idecdrompin.exp
idecdrompin_EXPORTS	= -bE:idecdrompin.exp

idecdrom_EXPORTS	= -bE:idecdrom.exp
idecdrompin_IMPORTS	= -bI:idecdrom.exp

idecdrom_OFILES = idecdromt.o

idecdrompin_OFILES = idecdromb.o

CFLAGS                  = -D_KERNEL -DPM_SUPPORT

STRIP_FLAG ?= -s

LDFLAGS_KERNEXT += ${STRIP_FLAG}

root/devices.ide.cdrom.rte.trc.S: devices.ide.cdrom.rte.trc.S
	-${MKDIR} root
	${CP} ${devices.ide.cdrom.rte.trc.S:P} \
		root/devices.ide.cdrom.rte.trc.S

.include <${RULES_MK}>
