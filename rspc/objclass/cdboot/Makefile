# @(#)35    1.36  src/rspc/objclass/cdboot/Makefile, bosboot, rspc41J, 9520B_all  5/19/95  10:35:36
# COMPONENT_NAME: BOSBOOT
#
# FUNCTIONS:
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
# This Makefile creates the ODM that resides in the RAM filesystem
# during CDROM boot.  This ODM should contain only the .add files for
# CDROM, disk, display, tape, diskette devices and their parent devices.

VPATH =		./:../:../${TARGET_MACHINE}:${MAKETOP}/bos/objclass:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

CD_LIST =	\
		3353c088.add \
		3353c188.add \
		3353c288.add \
		3353c388.add \
		adapter.isa.sio.add \
		adapter.isa_sio.IBM0005.IBM8301.add \
		adapter.isa_sio.fda.add \
		adapter.isa_sio.ide.add \
		adapter.isa_sio.isa_keyboard.add \
		adapter.isa_sio.isa_mouse.add \
		adapter.isa_sio.s1a.add \
		adapter.isa_sio.s2a.add \
		adapter.pci.ncr810.add \
		adapter.pci.ncr825.add \
		bbl.pci.add \
		bus.pci.pci.add \
		bus.pci.isa.add \
		bus.sys.pci.add \
		cfgrule.lft.add \
		cfgrule.lvm.add \
		cfgrule.sys.add \
		cfgrule.tty.add \
		devno.add \
		disk.ide.1080mbDPEA.add \
		disk.ide.540mbDALA.add \
		disk.ide.540mb.add \
		disk.ide.540mbWD.add \
		disk.ide.720mb.add \
		disk.ide.730mbWD.add \
		disk.ide.730mb.add \
		disk.ide.oidisk.add \
		disk.scsi.1000mb.add \
		disk.scsi.1000mb16bit.add \
		disk.scsi.1000mb16bitde.add \
		disk.scsi.1000mb2.add \
		disk.scsi.1000mbde.add \
		disk.scsi.1080mb.add \
		disk.scsi.1100mb.add \
		disk.scsi.1100mb16bit.add \
		disk.scsi.1100mb16bitde.add \
		disk.scsi.1200mb.add \
		disk.scsi.1370mb.add \
		disk.scsi.160mb.add \
		disk.scsi.2000mb.add \
		disk.scsi.2000mb16bit.add \
		disk.scsi.2000mb16bitde.add \
		disk.scsi.2000mbde.add \
		disk.scsi.200mb.add \
		disk.scsi.2200mb.add \
		disk.scsi.2200mb16bit.add \
		disk.scsi.2200mb16bitde.add \
		disk.scsi.270mb.add \
		disk.scsi.270mb2.add \
		disk.scsi.320mb.add \
		disk.scsi.340mb.add \
		disk.scsi.355mb.add \
		disk.scsi.360mb.add \
		disk.scsi.400mb.add \
		disk.scsi.4500mb16bit.add \
		disk.scsi.4500mb16bitde.add \
		disk.scsi.540mb.add \
		disk.scsi.540mb2.add \
		disk.scsi.540mb3.add \
		disk.scsi.540mb4.add \
		disk.scsi.540mb5.add \
		disk.scsi.670mb.add \
		disk.scsi.730mb.add \
		disk.scsi.730mb2.add \
		disk.scsi.810mb.add \
		disk.scsi.857mb.add \
		disk.scsi.osdisk.add \
		disk.scsi.scsd.add \
		diskette.siofd.fd.add \
		lft.add \
		logvol.add \
		lvm.add \
		memory.sys.L2cache.add \
		memory.sys.L2cache_rspc.add \
		memory.sys.simm.add \
		memory.sys.totmem.add \
		pcifga.add \
		pcigga.add \
		pcihga.add \
		pciwfg.add \
		planar.sys.sysplanar_rspc.add \
		processor.sys.proc_rspc.add \
		rwoptical.scsi.scsd.add \
		std_keyboard.add \
		sys.node.rspc.add \
		tape.scsi.1200mb-c.add \
		tape.scsi.150mb.add \
		tape.scsi.4mm2gb.add \
		tape.scsi.4mm2gb2.add \
		tape.scsi.525mb.add \
		tape.scsi.8mm.add \
		tape.scsi.8mm5gb.add \
		tape.scsi.8mm7gb.add \
		tape.scsi.9trk.add \
		tape.scsi.ost.add \
		tty.add

BUILD_LIST =	Config_Rules \
		CuAt \
		CuAt.vc \
		CuDep \
		CuDv \
		CuDvDr \
		CuVPD \
		PdAt \
		PdAt.vc \
		PdCn \
		PdDv \
		PdDv.vc

CHANGE_CD_LIST	= \
		cdrom.ide.cdrom1.add \
		cdrom.ide.cdrom4X.add \
		cdrom.ide.oicdrom.add \
		cdrom.scsi.cdrom1.add \
		cdrom.scsi.enhcdrom.add \
		cdrom.scsi.enhcdrom2.add \
		cdrom.scsi.enhcdrom3.add \
		cdrom.scsi.enhcdrom4.add \
		cdrom.scsi.oscd.add \
		cdrom.scsi.scsd.add

NEW_CD_LIST	= \
		cdrom.ide.cdrom1.add.new \
		cdrom.ide.cdrom4X.add.new \
		cdrom.ide.oicdrom.add.new \
		cdrom.scsi.cdrom1.add.new \
		cdrom.scsi.enhcdrom.add.new \
		cdrom.scsi.enhcdrom2.add.new \
		cdrom.scsi.enhcdrom3.add.new \
		cdrom.scsi.enhcdrom4.add.new \
		cdrom.scsi.oscd.add.new \
		cdrom.scsi.scsd.add.new

ILIST		= ${BUILD_LIST}

IDIR		= /tmp/rspc.cdboot_IBM/

OTHERS	= eject_fix temp

NOUPDATE_LIST	= ${BUILD_LIST:@TMP@${IDIR}${TMP}@}

.include <${RULES_MK}>


eject_fix : ${CHANGE_CD_LIST}
	# Below is fix for cd eject to work, along with add a ./: infront of VPATH
	$(RM) -f ${.TARGET} sedscript ${NEW_CD_LIST}
	for i in `$(ECHO) ${CHANGE_CD_LIST:@TMP@${${TMP}:P}@}`; do if [ ! -f `$(BASENAME) $$i` ]; then $(CP) $$i .; fi;$(ECHO) "$(SED) \"/prevent_eject/{\nn\n/deflt/s/.deflt.=.yes/deflt = no/\n}\" `$(BASENAME) $$i` > `$(BASENAME) $$i.new`" >> sedscript; done
	/bin/sh ./sedscript
	${TOUCH} ${.TARGET}


temp	: ${CD_LIST} cfgodm.cre eject_fix
	${RM} -f temp cfgodm.ipl cfgodm.fdb
	${CP} ${cfgodm.cre:P} .
	ODMDIR=. ${ODMCREATE} -c -p cfgodm.cre \
	|| { ${RM} -f cfgodm.cre; ${FALSE} ; }
	${RM} -f cfgodm.cre
	ODMDIR=. ${ODMADD} ${CD_LIST:@TMP@${${TMP}:P}@} ${NEW_CD_LIST:@TMP@${${TMP}:P}@} > /dev/null \
	|| { ${RM} -f ${BUILD_LIST}; ${FALSE} ; }
	${TOUCH} temp
