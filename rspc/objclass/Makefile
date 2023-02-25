# @(#)63    1.28  src/rspc/objclass/Makefile, rspccfg, rspc41J, 9520A_all  5/12/95  12:22:04
#
# COMPONENT_NAME: (CFGMETHODS) Makefiles
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
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
# LEVEL 1, 5 Years Bull Confidential Information
#

VPATH		= ${TARGET_MACHINE}:${MAKETOP}/bos/objclass:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

POWER_SUBDIRS	= cdboot tapeboot

SUBDIRS	= ${${TARGET_MACHINE}_SUBDIRS}

ILIST		=  \
		3353c088.add \
		3353c188.add \
		3353c288.add \
		3353c388.add \
		adapter.isa.c1x.add \
		adapter.isa.sio.add \
		adapter.isa_sio.fda.add \
		adapter.isa_sio.isa_keyboard.add \
		adapter.isa_sio.isa_mouse.add \
		adapter.isa_sio.ppa.add \
		adapter.isa_sio.s1a.add \
		adapter.isa_sio.s2a.add \
		adapter.pci.ncr810.add \
		adapter.pci.ncr825.add \
		adapter.pcmcia.a4000200.add \
		adapter.pcmcia.a4001d00.add \
		adapter.pcmcia.a4001e00.add \
		bus.isa.pcmcia.add \
		bus.pci.isa.add \
		bus.pci.pci.add \
		bus.sys.pci.add \
		cdrom.scsi.enhcdrom2.add \
		darticdd_isa.add \
		disk.scsi.1000mb2.add \
		disk.scsi.1080mb.add \
		disk.scsi.270mb.add \
		disk.scsi.270mb2.add \
		disk.scsi.340mb.add \
		disk.scsi.360mb.add \
		disk.scsi.540mb2.add \
		disk.scsi.540mb3.add \
		disk.scsi.540mb4.add \
		disk.scsi.540mb5.add \
		disk.scsi.730mb2.add \
		disk.scsi.810mb.add \
		pcifga.add \
		pcigga.add \
		pcihga.add \
		pcivca.add \
		pciwfg.add \
		planar.sys.sysplanar_rspc.add \
		sm_vca.add \
		sys.node.rspc.add \
		tape.scsi.4mm2gb2.add

IDIR		= /etc/stanzas/

NOUPDATE_LIST	= ${ILIST:@TMP@${${TMP}_IDIR:U${IDIR}}${TMP}@}

.include <${RULES_MK}>
