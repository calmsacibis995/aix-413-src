# @(#)57	1.53  src/packages/bos/diagboot/Makefile, diagboot, pkg41J, 9525A_all 6/16/95 16:45:31
#
# COMPONENT_NAME: DIAGBOOT
#
# FUNCTIONS: create diagnostics boot images
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
#
# (C) COPYRIGHT International Business Machines Corp. 1993, 1995
# All Rights Reserved
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/objclass:${MAKETOP}/bos/objclass/${TARGET_MACHINE}:${MAKETOP}/bos/diag/boot:${MAKETOP}/rspc/objclass:${MAKETOP}/rspc/objclass/${TARGET_MACHINE}

# Add ${MAKETOP}/ssa/objclass/${TARGET_MACHINE} and
# ${MAKETOP}/ssa/diag/util/umayf/odm to get ssa add file.

SUBDIRS		= supplemental

BOOTSCRIPTS	= rc.boot flattendb rmfiles restore_files term.dep

# NOTE: tapediag*.dep are picked up during the building of the tape RAM
# file system and are used by the diagnostic controller to control the
# number of file to read in.
# tape*.dep files are used to build the various backup images.

SCRIPTS		= cdrom.proto rspc.cdrom.proto tape.proto proto.awk \
		  copyshipfile.awk cdfs.dep \
		  diagcfg.add tapediag3A.dep tapediag3B.dep tapediag3C.dep \
		  tapediag5.dep tapediag6A.dep tapediag6B.dep tapediag7A.dep \
		  tapediag7B.dep tapediag8.dep tape1.dep tape3.dep \
		  tape5.dep tape6.dep tape7.dep tape8.dep linkedfiles.dep \
		  base.dep config1.dep config2.dep config3.dep config4.dep \
		  config5.dep graphics1.dep graphics2.dep graphics3.dep \
		  removed_graphics.dep \
		  createshipfile.awk .version ${BOOTSCRIPTS}

BOOTLIBS	= libodm.a libcfg.a

OTHERLIBS	= libdiag.a libcur.a libasl.a

ADDLIST 	= sys.node.sys.add sys.node.sys1.add sys.node.sys_p.add \
		  devno.add bus.sys.mca.add memory.sys.L2cache.add \
		  cpucard.sys.cpucard1.add processor.sys.proc1.add \
		  memory.sys.memory.add memory.sys.simm.add \
		  planar.sys.sysplanar1.add planar.sys.sysplanar2.add \
		  planar.sys.sysplanar3.add planar.sys.sysplanar_p.add \
		  sysunit.sys.sysunit.add \
		  container.sys.cabinet1.add \
		  container.cabinet.sif1.add \
		  container.cabinet.power_supply1.add \
		  container.cabinet.op_panel1.add \
		  ioplanar.sys.iodplanar1.add ioplanar.planar.mcaplanar1.add \
		  ioplanar.sys.ioplanar.add \
		  ioplanar.sys.ioplanar_1.add \
		  ioplanar.sys.ioplanar_2.add \
		  adapter.sio.s1a.add adapter.sio.s2a.add \
		  adapter.sio.s3a.add adapter.mca.sio.add \
		  adapter.mca.sio_1.add adapter.mca.sio_2.add \
		  adapter.mca.sio_3.add adapter.sio.hscsi.add \
		  adapter.mca.hscsi.add adapter.mca.8ff4.add \
		  adapter.mca.8fba.add adapter.sio.8efc.add \
		  adapter.mca.8efc.add driver.scsi_scb.vscsi.add \
		  adapter.sio.ppa.add adapter.sio.ppa_iod.add \
		  adapter.sio.fda.add adapter.sio.fda_2.add \
		  adapter.sio.fda_iod.add diskette.siofd.fd.add

RSPC_ADDLIST	= bus.pci.isa.add \
		  bbl.pci.add \
		  bus.pci.pci.add \
		  bus.sys.pci.add \
		  planar.sys.sysplanar_rspc.add \
		  processor.sys.proc_rspc.add \
		  memory.sys.L2cache.add \
		  memory.sys.L2cache_rspc.add \
		  memory.sys.totmem.add \
		  memory.sys.simm.add \
		  sys.node.rspc.add \
		  devno.add \
		  adapter.isa.sio.add \
		  adapter.isa_sio.IBM001C.add \
		  adapter.isa_sio.fda.add \
		  adapter.isa_sio.s1a.add \
		  adapter.isa_sio.s2a.add \
		  adapter.isa_sio.ppa.add \
		  adapter.pci.ncr810.add \
		  adapter.pci.ncr825.add \
		  diskette.siofd.fd.add \
		  cdrom.scsi.enhcdrom2.add \
		  cdrom.scsi.enhcdrom4.add


CDADDLIST	= cdrom.scsi.enhcdrom.add cdrom.scsi.cdrom1.add \
		  cdrom.scsi.enhcdrom3.add cdrom.scsi.scsd.add \
		  cdrom.scsi.oscd.add

TAPEADDLIST	= tape.scsi.150mb.add tape.scsi.4mm2gb.add \
		  tape.scsi.4mm4gb.add tape.scsi.525mb.add \
		  tape.scsi.8mm.add tape.scsi.8mm5gb.add \
		  tape.scsi.8mm7gb.add tape.scsi.9trk.add \
		  tape.scsi.3490e.add tape.scsi.1200mb-c.add \
		  tape.scsi.ost.add
# Tape for sandlefoot is tape.scsi.4mm2gb2.add

# This needs to contain everything needed to run from mounted
# CDROM as well. For example, driver uniquetype stanzas that
# cannot be merged. That's why fddi.add and sol.add are built here.

DIAGADDLIST	= DSMOpt.add DSMenu.add drawer.add \
		  diag.4001.add diag.4005.add \
		  diag.8d77.add diag.8ee3.add diag.8787.add \
		  diag.8ee4.add diag.8ee5.add diag.8ef2.add diag.8ffd.add \
		  diag.8f9a.add diag.8ef3.add diag.8ef4.add diag.8ef5.add \
		  diag.8efc.add diag.8f70.add diag.8f78.add diag.8f98.add \
		  diag.8fba.add diag.8fbc.add diag.8fc3.add diag.8fc8.add \
		  diag.8fa2.add diag.8ff4.add diag.base.add sol.add fddi.add \
		  diag.df9f.add diag.e1ff.add diag.edd0.add diag.deff.add \
		  diag.edd1.add diag.edd2.add diag.edd3.add diag.edd5.add \
		  diag.edd6.add diag.eff0.add diag.fda.add diag.fe92.add \
		  diag.ffe1.add diag.gio.add diag.ktma.add diag.mcakm.add \
		  diag.ppa.add diag.sa.add diag.scarray.add \
		  diag.scsicdrm.add diag.scsidisk.add \
		  diag.scsitape.add diag.sga.add diag.slc.add \
		  diag.wga.add printer.opp.add darticdd.add \
		  diag.4006.add diag.4007.add diag.efc6.add \
		  diag.4002.add diag.disksa.add printer.opp.diag.add \
		  diag.rspcbase.add diag.isasrvaid.add \
		  diag.isafd.add diag.isakm.add diag.isappa.add \
		  diag.isasa.add diag.14101800.add \
		  diag.22100020.add diag.00100300.add diag.0e100091.add \
		  diag.pnp0401.add diag.pcxr.add diag.cxia128.add

ASYNCADDLIST	= tty.add \
		  adapter.mca.8p188.add adapter.mca.8p232.add \
		  adapter.mca.8p422.add adapter.mca.16p232.add \
		  adapter.mca.16p422.add

COMMOADDLIST	= ent.add 3270.add tok.add ampx.add \
		  mpqp.add apm.add cat.add mpa.add mpaa.add \
		  msla.add psla.add ssla.add \
		  kent.add ient_1.add ient_2.add ient_6.add mps_tok.add \
		  stok.add

# ISA devices needs to be built together and added to ODM
# after calling instdbcln

ISAADDLIST	= pcxr.add cxia.add

# For devices having ROM scanning capability, there is no need to
# include their .add file here.

DISPLAYADDLIST	= skycolor.add skymono.add sgabus.add sga.add gemini.add \
		  mid.add nep.add \
		  pcifga.add pcigga.add pcivca.add pciwfg.add

INPUTDEVADDLIST	= gio.add sgio.add lft.add mouse.add kbd.add kts.add rcm.add \
		  tablet.add mca_kma.add std_keyboard.add \
		  std_mouse.add std_tablet.add \
		  adapter.isa_sio.isa_mouse.add \
		  adapter.isa_sio.isa_keyboard.add

MEDIAADDLIST	= disk.mca.badisk.add adapter.mca.serdasda.add

DISKSADDLIST	= disk.scsi.1000mb.add disk.scsi.1000mbde.add \
		  disk.scsi.1000mb16bit.add disk.scsi.1000mb16bitde.add \
		  disk.scsi.2000mb16bitde.add disk.scsi.2000mbde.add \
		  disk.scsi.2000mb16bit.add disk.scsi.200mb.add \
		  disk.scsi.1200mb.add disk.scsi.1370mb.add \
		  disk.scsi.160mb.add disk.scsi.2000mb.add \
		  disk.scsi.320mb.add disk.scsi.355mb.add disk.scsi.400mb.add \
		  disk.scsi.540mb.add disk.scsi.670mb.add \
		  disk.scsi.857mb.add disk.scsi.osdisk.add \
		  adapter.serdasda.serdasdc.add disk.serdasdc.serdasdd.add \
		  disk.serdasdc.serdasdd2.add disk.serdasdc.serdasdd4.add \
		  disk.scsi.scsd.add disk.serdasdc.2000mb.add \
		  disk.scsi.1100mb.add disk.scsi.1100mb16bit.add \
		  disk.scsi.1100mb16bitde.add disk.scsi.2200mb.add \
		  disk.scsi.2200mb16bit.add disk.scsi.2200mb16bitde.add \
		  disk.scsi.4500mb16bit.add disk.scsi.4500mb16bitde.add
# Disks supported on Sandlefoot
#		  disk.scsi.1000mb2.add disk.scsi.270mb.add \
#		  disk.scsi.270mb2.add \
#		  disk.scsi.340mb.add disk.scsi.360mb.add \
#		  disk.scsi.540mb2.add disk.scsi.540mb3.add \
#		  disk.scsi.540mb4.add disk.scsi.540mb5.add \
#		  disk.scsi.730mb2.add disk.scsi.810mb.add
#
TAPE_IMAGES	= tape1.image tape3.image tape5.image tape6.image \
		  tape7.image tape8.image

CLEANFILES	= ${BOOTLIBS} ${OTHERLIBS} buildtape.proto diagboot.tape \
		  tape.fs buildcd.proto diagboot.cd cdrom.fs \
		  rspc.buildcd.proto rspc.diagboot.cd rspc.cdrom.fs

OTHERS1		= temp tempdiag term.cpio cdfiles.add tapefiles.add \
		  rspc.cdfiles.add cdrom.add tapes.add \
		  diagodm.add ${ODM_ADD} ${TAPE_IMAGES} cdfs.cpio

OTHERS		= ${OTHERS1} diagboot.cd diagboot.tape rspc.diagboot.cd

ODM_ADD		= commo.add media.add async.add inputdev.add disks.add \
		  display.add isadev.add

.include <${RULES_MK}>

# Creating empty ODM object class

temp    : cfgodm.cre
	${RM} -f temp cfgodm.ipl cfgodm.fdb
	${CP} ${cfgodm.cre:P} .
	ODMDIR=. ${ODMCREATE} -c -p cfgodm.cre
	${RM} -f cfgodm.cre
	${TOUCH} temp

tempdiag : diagodm.cre
	${RM} -f tempdiag diagodm.cre
	${CP} ${diagodm.cre:P} .
	ODMDIR=. ${ODMCREATE} -c -p diagodm.cre
	${CHMOD} +r diagodm.cre
	${RM} -f diagodm.cre
	${TOUCH} tempdiag

# Gathering terminal dependency file and put all in
# one cpio file.

term.cpio: term.dep copyshipfile.awk
	${RM} -f ${.TARGET}
	${CAT} term.dep | ${AWK} -f ${copyshipfile.awk:P}
	${CPIO} -oC36 <term.dep > ${.TARGET} || ${RM} -rf ${.TARGET}
	for i in `${CAT} term.dep` ; do \
	${RM} -rf $$i; \
	done;

# Build add files used in CDROM diagnostics.
# diagcfg.add is built in this directory, therefore does not
# need to be copied over from the ship/source tree.

cdfiles.add: ${ADDLIST} ${CDADDLIST} diagcfg.add
	${RM} -f ${.TARGET}
	(for i in ${.ALLSRC} ; do if [ "$$i" != "diagcfg.add" ]; \
	then ${CP} $$i . ; fi; done)
	${CAT} diagcfg.add >> ${.TARGET} || { ${RM} -f ${.TARGET} ; ${FALSE} ; }
	(for i in ${ADDLIST} ; do ${CAT} $$i >> ${.TARGET}; done) \
	|| { ${RM} -f templist ${ADDLIST} ; ${FALSE} ;}
	(for i in ${CDADDLIST} ; do ${CAT} $$i >> ${.TARGET}; done) \
	|| { ${RM} -f ${.TARGET} ${CDADDLIST} ; ${FALSE} ;}
	${RM} -f ${ADDLIST}
	${RM} -f ${CDADDLIST}
	${TOUCH} cdfiles.add


# Build add files used in RSPC CDROM diagnostics.
# diagcfg.add is built in this directory, therefore does not
# need to be copied over from the ship/source tree.

rspc.cdfiles.add: ${RSPC_ADDLIST} ${CDADDLIST} diagcfg.add
	${RM} -f ${.TARGET}
	(for i in ${.ALLSRC} ; do if [ "$$i" != "diagcfg.add" ]; \
	then ${CP} $$i . ; fi; done)
	${CAT} diagcfg.add >> ${.TARGET} || { ${RM} -f ${.TARGET} ; ${FALSE} ; }
	(for i in ${RSPC_ADDLIST} ; do ${CAT} $$i >> ${.TARGET}; done) \
	|| { ${RM} -f templist ${RSPC_ADDLIST} ; ${FALSE} ;}
	(for i in ${CDADDLIST} ; do ${CAT} $$i >> ${.TARGET}; done) \
	|| { ${RM} -f ${.TARGET} ${CDADDLIST} ; ${FALSE} ;}
	${RM} -f ${RSPC_ADDLIST}
	${RM} -f ${CDADDLIST}
	${TOUCH} rspc.cdfiles.add


# Build add files used in TAPE diagnostics.
# diagcfg.add is built in this directory, therefore does not
# need to be copied over from the ship/source tree.

tapefiles.add: ${ADDLIST} ${TAPEADDLIST} diagcfg.add
	${RM} -f ${.TARGET}
	(for i in ${.ALLSRC} ; do if [ "$$i" != "diagcfg.add" ]; \
	then ${CP} $$i . ; fi; done)
	${CAT} diagcfg.add >> ${.TARGET} || { ${RM} -f ${.TARGET} ; ${FALSE} ; }
	(for i in ${ADDLIST} ; do ${CAT} $$i >> ${.TARGET}; done) \
	|| { ${RM} -f ${.TARGET} ; ${FALSE} ;}
	(for i in ${TAPEADDLIST} ; do ${CAT} $$i >> ${.TARGET}; done) \
	|| { ${RM} -f ${.TARGET} ${TAPEADDLIST} ; ${FALSE} ;}
	${RM} -f ${ADDLIST}
	${RM} -f ${TAPEADDLIST}
	${TOUCH} tapefiles.add

# Build add files containing stanzas needed to configure
# CDROM devices. This file will be used in TAPE diagnostics.

cdrom.add: ${CDADDLIST}
	${RM} -f ${.TARGET}
	(for i in ${.ALLSRC} ; do ${CP} $$i . ; done)
	(for i in ${CDADDLIST} ; do ${CAT} $$i >> ${.TARGET}; done) \
	|| { ${RM} -f ${.TARGET} ${CDADDLIST} ; ${FALSE} ;}
	${RM} -f ${CDADDLIST}
	${TOUCH} cdrom.add

# Build add files containing stanzas needed to configure
# TAPE devices. This file will be used in CDROM diagnostics.

tapes.add: ${TAPEADDLIST}
	${RM} -f ${.TARGET}
	(for i in ${.ALLSRC} ; do ${CP} $$i . ; done)
	(for i in ${TAPEADDLIST} ; do ${CAT} $$i >> ${.TARGET}; done) \
	|| { ${RM} -f ${.TARGET} ${TAPEADDLIST} ; ${FALSE} ;}
	${RM} -f ${TAPEADDLIST}
	${TOUCH} tapes.add

# Build add files containing all diagnostics stanzas

diagodm.add: ${DIAGADDLIST}
	${RM} -f ${.TARGET} templist
	(for i in ${.ALLSRC} ; do ${CP} $$i . ; done)
	(for i in ${DIAGADDLIST} ; do ${CAT} $$i >> templist; done) \
	|| { ${RM} -f templist ${DIAGADDLIST} ; ${FALSE} ;}
	${RM} -f ${DIAGADDLIST}
	${SED} -e '/^#/d' templist > ${.TARGET} || { ${RM} -f ${.TARGET}; ${FALSE} ;}
	${RM} templist
	${TOUCH} diagodm.add

commo.add: ${COMMOADDLIST}
	${RM} -f ${.TARGET} templist
	(for i in ${.ALLSRC} ; do ${CP} $$i . ; done)
	(for i in ${COMMOADDLIST} ; do ${CAT} $$i >> templist; done) \
	|| { ${RM} -f templist ${COMMOADDLIST} ; ${FALSE} ;}
	${RM} -f ${COMMOADDLIST}
	${SED} -e '/^#/d' templist > ${.TARGET} || { ${RM} -f ${.TARGET}; ${FALSE} ;}
	${RM} -f templist
	${TOUCH} commo.add

isadev.add: ${ISAADDLIST}
	${RM} -f ${.TARGET} templist
	(for i in ${.ALLSRC} ; do ${CP} $$i . ; done)
	(for i in ${ISAADDLIST} ; do ${CAT} $$i >> templist; done) \
	|| { ${RM} -f templist ${ISAADDLIST} ; ${FALSE} ;}
	${RM} -f ${ISAADDLIST}
	${SED} -e '/^#/d' templist > ${.TARGET} || { ${RM} -f ${.TARGET}; ${FALSE} ;}
	${RM} -f templist
	${TOUCH} isadev.add

inputdev.add: ${INPUTDEVADDLIST}
	${RM} -f ${.TARGET} templist
	(for i in ${.ALLSRC} ; do ${CP} $$i . ; done)
	(for i in ${INPUTDEVADDLIST} ; do ${CAT} $$i >> templist; done) \
	|| { ${RM} -f templist ${INPUTDEVADDLIST} ; ${FALSE} ;}
	${RM} -f ${INPUTDEVADDLIST}
	${SED} -e '/^#/d' templist > ${.TARGET} || { ${RM} -f ${.TARGET}; ${FALSE} ;}
	${RM} -f templist
	${TOUCH} inputdev.add

display.add: ${DISPLAYADDLIST}
	${RM} -f ${.TARGET} templist
	(for i in ${.ALLSRC} ; do ${CP} $$i . ; done)
	(for i in ${DISPLAYADDLIST} ; do ${CAT} $$i >> templist; done) \
	|| { ${RM} -f templist ${DISPLAYADDLIST} ; ${FALSE} ;}
	${RM} -f ${DISPLAYADDLIST}
	${SED} -e '/^#/d' templist > ${.TARGET} || { ${RM} -f ${.TARGET}; ${FALSE} ;}
	${RM} -f templist
	${TOUCH} display.add

media.add: ${MEDIAADDLIST}
	${RM} -f ${.TARGET} templist
	(for i in ${.ALLSRC} ; do ${CP} $$i . ; done)
# Update the adapter.mca.8f97.add to remove the Unconfigure
# method. This allow the adapter to stay available at all
# time.
	(for i in ${MEDIAADDLIST} ; do \
	if [ "$$i" = "adapter.mca.8f97.add" ]; \
	then $(SED) -e 's/\/usr\/lib\/methods\/ucfgdevice//' \
		adapter.mca.8f97.add > \
	adapter.mca.8f97.add.new; $(RM) -f adapter.mca.8f97.add; \
	$(MV) adapter.mca.8f97.add.new adapter.mca.8f97.add; \
	fi; \
	${CAT} $$i >> templist; done) \
	|| { ${RM} -f templist ${MEDIAADDLIST} ; ${FALSE} ;}
	${RM} -f ${MEDIAADDLIST}
	${SED} -e '/^#/d' templist > ${.TARGET} || { ${RM} -f ${.TARGET}; ${FALSE} ;}
	${TOUCH} media.add

disks.add: ${DISKSADDLIST}
	${RM} -f ${.TARGET} templist
	(for i in ${.ALLSRC} ; do ${CP} $$i . ; done)
	(for i in ${DISKSADDLIST} ; do ${CAT} $$i >> templist; done) \
	|| { ${RM} -f templist ${DISKSADDLIST} ; ${FALSE} ;}
	${RM} -f ${DISKSADDLIST}
	${SED} -e '/^#/d' templist > ${.TARGET} || { ${RM} -f ${.TARGET}; ${FALSE} ;}
	${RM} -f templist
	${TOUCH} disks.add

async.add: ${ASYNCADDLIST}
	${RM} -f ${.TARGET} templist
	(for i in ${.ALLSRC} ; do ${CP} $$i . ; done)
	(for i in ${ASYNCADDLIST} ; do ${CAT} $$i >> templist; done) \
	|| { ${RM} -f templist ${ASYNCADDLIST} ; ${FALSE} ;}
	${RM} -f ${ASYNCADDLIST}
	${SED} -e '/^#/d' templist > ${.TARGET} || { ${RM} -f ${.TARGET}; ${FALSE} ;}
	${TOUCH} async.add

# Strip libraries

${BOOTLIBS} ${OTHERLIBS}:
	${RM} -f shr.o ${.TARGET}
	if [ ${.TARGET} = "libcur.a" ] ; \
	then ${AR} -x `${ECHO} /usr/lib/${.TARGET} | ${FINDSHIPFILE}` \
	shr.o shr32c.o; ${STRIP} shr.o ; ${STRIP} shr32c.o ; \
	${AR} cq ${.TARGET} shr.o shr32c.o ; ${RM} shr.o shr32c.o ; \
	else ${AR} -x `${ECHO} /usr/lib/${.TARGET} | ${FINDSHIPFILE}` shr.o; \
	${STRIP} shr.o ; ${AR} cq ${.TARGET} shr.o ; ${RM} shr.o ; fi

bootrec:
	${TOUCH} ${.TARGET}

buildcd.proto: cdrom.proto proto.awk ${BOOTLIBS} ${BOOTSCRIPTS} ${PROGRAMS} \
		${OTHERS1}
	${AWK} -f ${proto.awk:P} < ${cdrom.proto:P} > buildcd.proto

rspc.buildcd.proto: rspc.cdrom.proto proto.awk ${BOOTLIBS} \
			${BOOTSCRIPTS} ${PROGRAMS} ${OTHERS1}
	${AWK} -f ${proto.awk:P} < ${rspc.cdrom.proto:P} > rspc.buildcd.proto

buildtape.proto: tape.proto proto.awk ${BOOTLIBS} ${OTHERLIBS} ${BOOTSCRIPTS} \
		${PROGRAMS} ${OTHERS1}
	${AWK} -f ${proto.awk:P} < ${tape.proto:P} > buildtape.proto

cdrom.fs: buildcd.proto bootrec
	${TOUCH} ${.TARGET}
	${ECHO} y | ${MKFS} -V jfs -p ${buildcd.proto:P} ${.TARGET}

rspc.cdrom.fs: rspc.buildcd.proto bootrec
	${TOUCH} ${.TARGET}
	${ECHO} y | ${MKFS} -V jfs -p ${rspc.buildcd.proto:P} ${.TARGET}

tape.fs:	buildtape.proto bootrec
	${TOUCH} ${.TARGET}
	${ECHO} y | ${MKFS} -V jfs -p ${buildtape.proto:P} ${.TARGET}

cdrom.ramfs:	cdrom.fs
	${MKRAM} ${cdrom.fs:P} ${.TARGET}

rspc.cdrom.ramfs:	rspc.cdrom.fs
	${MKRAM} ${rspc.cdrom.fs:P} ${.TARGET}

tape.ramfs:	tape.fs
	${MKRAM} ${tape.fs:P} ${.TARGET}

diagboot.cd: cdrom.ramfs
	${TOUCH} /tmp/cdboot.build_dev
	${MKBOOT} -d/tmp/cdboot.build_dev -k`${ECHO} /unix_mp.strip | \
		${FINDSHIPFILE}` -f/tmp/cdboot.build_dev | ${COMPRESS} > \
		/tmp/cdboot.build_image
	${CAT} ${cdrom.ramfs:P} >> /tmp/cdboot.build_image
	${MKBOOT} -d/tmp/cdboot.build_image -bisk \
		`${ECHO} /usr/sbin/bootexpand | ${FINDSHIPFILE}` \
		-f/tmp/cdboot.build_image > diagboot.cd
	${RM} /tmp/cdboot.build_image /tmp/cdboot.build_dev

rspc.diagboot.cd: rspc.cdrom.ramfs
	${TOUCH} /tmp/rspc.cdboot.build_dev
	${MKBOOT} -d/tmp/rspc.cdboot.build_dev -k`${ECHO} /unix_up.strip | \
		${FINDSHIPFILE}` -f/tmp/rspc.cdboot.build_dev | ${COMPRESS} > \
		/tmp/rspc.cdboot.build_image
	${CAT} ${rspc.cdrom.ramfs:P} >> /tmp/rspc.cdboot.build_image
	${MKBOOT} -d/tmp/rspc.cdboot.build_image -bisk \
		`${ECHO} /usr/sbin/bootexpand | ${FINDSHIPFILE}` \
		-A `${ECHO} /usr/lib/boot/aixmon | ${FINDSHIPFILE}` \
		-f/tmp/rspc.cdboot.build_image > rspc.diagboot.cd
	${RM} /tmp/rspc.cdboot.build_image /tmp/rspc.cdboot.build_dev

# Use MKBOOT with -I to invoke kernel debugger at boot time.
# Otherwise use the -D. Shipped CD/TAPE will not have kernel
# debugger loaded.

diagboot.tape: tape.ramfs
	${TOUCH} /tmp/tapeboot.build_dev
	${MKBOOT} -d/tmp/tapeboot.build_dev -k`${ECHO} /unix_mp.strip | \
		${FINDSHIPFILE}` -f/tmp/tapeboot.build_dev | ${COMPRESS} > \
		/tmp/tapeboot.build_image
	${CAT} ${tape.ramfs:P} >> /tmp/tapeboot.build_image
	${MKBOOT} -d/tmp/tapeboot.build_image -bisk \
		`${ECHO} /usr/sbin/bootexpand | ${FINDSHIPFILE}` \
		-f/tmp/tapeboot.build_image > diagboot.tape
	${RM} /tmp/tapeboot.build_image /tmp/tapeboot.build_dev


# Build a cpio file that will contain all the files needed
# to build the CDROM file system.

.undef	FOOLIST
cdfs.cpio: cdfs.dep \
	${FOOLIST:!if [ -f cdfs.dep ] ; then \
	for i in `${CAT} cdfs.dep`; do \
	${ECHO} `${ECHO} $$i | ${SED} -e 's,.,,' | ${FINDSHIPFILE}`; done; fi!} \
	copyshipfile.awk ${ODM_ADD} tapes.add diagodm.add \
	${OTHERLIBS} ${BOOTLIBS} .version
	${RM} -f ${.TARGET} templist
	${CP} cdfs.dep templist
	for i in ${OTHERLIBS} ${BOOTLIBS} ${ODM_ADD} \
		tapes.add diagodm.add .version ; do \
	${ECHO} "./"$$i >> templist; \
	done;
	${CAT} templist | ${AWK} -f ${copyshipfile.awk:P}
	${CPIO} -oC36 <templist > ${.TARGET} || { ${RM} -rf ${.TARGET}; ${FALSE} ;}
	for i in `${CAT} cdfs.dep` ; do \
	${RM} -rf $$i; \
	done;
	${RM} -f templist


.undef  DIAG_TAPE1_LIST
tape1.image: tape1.dep copyshipfile.awk \
	${DIAG_TAPE1_LIST:!if [ -f tape1.dep ] ; then \
	for i in `${CAT} tape1.dep`; do \
	${ECHO} `${ECHO} $$i | ${SED} -e 's,.,,' | ${FINDSHIPFILE}`; done; fi!}
	${CAT} tape1.dep | ${AWK} -f ${copyshipfile.awk:P}
	${CAT} tape1.dep | ${BACKUP} -ivrpf ${.TARGET} -b30 >/dev/null
	for i in `${CAT} tape1.dep` ; do \
	${RM} -rf $$i; \
	done;

.undef  DIAG_TAPE3_LIST
tape3.image: tape3.dep copyshipfile.awk \
	${DIAG_TAPE3_LIST:!if [ -f tape3.dep ] ; then \
	for i in `${CAT} tape3.dep`; do \
	${ECHO} `${ECHO} $$i | ${SED} -e 's,.,,' | ${FINDSHIPFILE}`; done; fi!}
	${CAT} tape3.dep | ${AWK} -f ${copyshipfile.awk:P}
	${CAT} tape3.dep | ${BACKUP} -ivrpf ${.TARGET} -b30 >/dev/null
	for i in `${CAT} tape3.dep` ; do \
	${RM} -rf $$i; \
	done;

.undef  DIAG_TAPE5_LIST
tape5.image: tape5.dep copyshipfile.awk \
	${DIAG_TAPE5_LIST:!if [ -f tape5.dep ] ; then \
	for i in `${CAT} tape5.dep`; do \
	${ECHO} `${ECHO} $$i | ${SED} -e 's,.,,' | ${FINDSHIPFILE}`; done; fi!}
	${CAT} tape5.dep | ${AWK} -f ${copyshipfile.awk:P}
	${CAT} tape5.dep | ${BACKUP} -ivrpf ${.TARGET} -b30 >/dev/null
	for i in `${CAT} tape5.dep` ; do \
	${RM} -rf $$i; \
	done;

.undef  DIAG_TAPE6_LIST
tape6.image: tape6.dep copyshipfile.awk \
	${DIAG_TAPE6_LIST:!if [ -f tape6.dep ] ; then \
	for i in `${CAT} tape6.dep`; do \
	${ECHO} `${ECHO} $$i | ${SED} -e 's,.,,' | ${FINDSHIPFILE}`; done; fi!}
	${CAT} tape6.dep | ${AWK} -f ${copyshipfile.awk:P}
	${CAT} tape6.dep | ${BACKUP} -ivrpf ${.TARGET} -b30 >/dev/null
	for i in `${CAT} tape6.dep` ; do \
	${RM} -rf $$i; \
	done;

.undef  DIAG_TAPE7_LIST
tape7.image: tape7.dep copyshipfile.awk \
	${DIAG_TAPE7_LIST:!if [ -f tape7.dep ] ; then \
	for i in `${CAT} tape7.dep`; do \
	${ECHO} `${ECHO} $$i | ${SED} -e 's,.,,' | ${FINDSHIPFILE}`; done; fi!}
	${CAT} tape7.dep | ${AWK} -f ${copyshipfile.awk:P}
	${CAT} tape7.dep | ${BACKUP} -ivrpf ${.TARGET} -b30 >/dev/null
	for i in `${CAT} tape7.dep` ; do \
	${RM} -rf $$i; \
	done;

.undef  DIAG_TAPE8_LIST
tape8.image: tape8.dep copyshipfile.awk \
	${DIAG_TAPE8_LIST:!if [ -f tape8.dep ] ; then \
	for i in `${CAT} tape8.dep`; do \
	${ECHO} `${ECHO} $$i | ${SED} -e 's,.,,' | ${FINDSHIPFILE}`; done; fi!}
	${CAT} tape8.dep | ${AWK} -f ${copyshipfile.awk:P}
	${CAT} tape8.dep | ${BACKUP} -ivrpf ${.TARGET} -b30 >/dev/null
	for i in `${CAT} tape8.dep` ; do \
	${RM} -rf $$i; \
	done;
