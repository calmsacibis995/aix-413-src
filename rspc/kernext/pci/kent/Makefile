# @(#)45        1.2  src/rspc/kernext/pci/kent/Makefile, pcient, rspc41J, 9515A_all 4/11/95 21:21:09
#
#   COMPONENT_NAME: PCIENT
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

EXPDIR		= /kernel/include/sys/

OTHERS          = kent_errids.h  \
		  root/devices.pci.22100020.rte.err.S  \
		  root/devices.pci.22100020.rte.trc.S

KERNEL_EXT 		= kentdd

IDIR 			= /usr/lib/drivers/pci/

ILIST                   = kentdd

kentdd_ENTRYPOINT 	= kent_config

LIBS			= ${KERNEXT_LIBS}

kentdd_OFILES = \
	kent_config.o	\
	kent_open.o	\
	kent_close.o	\
	kent_tx.o	\
	kent_rx.o	\
	kent_ctl.o	\
	kent_intr.o	\
	kent_limbo.o	\
	kent_dump.o	\
	kent_prim.o	\
	kent_pmh.o

kent_errids.h: devices.pci.22100020.rte.err.S
		${CP} ${devices.pci.22100020.rte.err.S:P} .
		${ERRTMPLTBLD} -d devices.pci.22100020.rte.err.S -o kent_errids.h
		${RM} -f devices.pci.22100020.rte.err.S

root/devices.pci.22100020.rte.err.S: devices.pci.22100020.rte.err.S
		-${MKDIR} root
		${CP} ${devices.pci.22100020.rte.err.S:P} root/devices.pci.22100020.rte.err.S

root/devices.pci.22100020.rte.trc.S: devices.pci.22100020.rte.trc.S
		-${MKDIR} root
		${CP} ${devices.pci.22100020.rte.trc.S:P} root/devices.pci.22100020.rte.trc.S

.include <${RULES_MK}>

