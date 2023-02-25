# "@(#)37  1.4  src/rspc/kernext/pci/stok/Makefile, pcitok, rspc41J, 9522A_all 5/30/95 19:02:40"
#
#   COMPONENT_NAME: pcitok
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

OTHERS          = tr_stok_errids.h \
                  root/devices.pci.14101800.rte.err.S \
                  root/devices.pci.14101800.rte.trc.S

KERNEL_EXT	= stokdd

IDIR		= /usr/lib/drivers/pci/

ILIST           = stokdd 

stokdd_ENTRYPOINT	= stok_config
LIBS			= ${KERNEXT_LIBS}  

${stokdd_OFILES}: tr_stok_errids.h

stokdd_OFILES = \
	stok_util.o stok_cfg.o stok_open.o stok_close.o stok_tx.o  \
        stok_rx.o stok_wdt.o stok_intr.o stok_function.o stok_ctl.o \
        stok_dump.o stok_pmh.o  

tr_stok_errids.h: stok.err.S
	 ${CP} ${stok.err.S:P} .
	 ${ERRTMPLTBLD} -d stok.err.S -o tr_stok_errids.h
	 ${RM} -f stok.err.S

root/devices.pci.14101800.rte.err.S: stok.err.S	
		-${MKDIR} root
		 ${CP} ${stok.err.S:P} root/devices.pci.14101800.rte.err.S

root/devices.pci.14101800.rte.trc.S: stok.trc.S
		 -${MKDIR} root
		 ${CP} ${stok.trc.S:P} root/devices.pci.14101800.rte.trc.S

.include <${RULES_MK}>
