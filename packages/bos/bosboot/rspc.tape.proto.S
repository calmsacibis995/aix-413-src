#if 0
# @(#)30  1.2  src/packages/bos/bosboot/rspc.tape.proto.S, bosboot, bos41J, 9519B_all 5/9/95 12:33:19
#
#   COMPONENT_NAME: BOSBOOT
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
#endif
/bootrec 10240 0
d--- 755 0 0
bin	l--- 777 0 0 /usr/bin
dev	d--- 755 0 0
	bus0	  c--- 640 0 0 3 0
	console   c--- 622 0 0 4 0
	error	  c--- 622 0 0 6 0
	errorctl  c--- 600 0 0 6 1
	kmem	  c--- 440 0 0 2 1
	mem	  c--- 440 0 0 2 0
	null	  c--- 666 0 0 2 2
	nvram	  c--t 640 0 0 3 0
	ram0	  b--- 600 0 0 0 0
	ram1	  b--- 600 0 0 0 1
	sysdump   c--- 600 0 0 7 0
	sysdumpctl  c--- 600 0 0 7 1
	sysdumpfile c--- 600 0 0 7 3
	sysdumpnull c--- 600 0 0 7 2
	tty	  c--- 666 0 0 1 0
	$
etc	d--- 755 0 0
	drivers	l--- 777 0 0 /usr/lib/drivers
	init	---- 777 0 0 /usr/lib/boot/ssh
	methods l--- 777 0 0 /usr/lib/methods
	microcode l--- 777 0 0 /usr/lib/microcode
	objrepos d--- 777 0 0
		Config_Rules	---- 777 0 0 /tmp/rspc.tapeboot_IBM/Config_Rules
		CuAt	---- 777 0 0 /tmp/rspc.tapeboot_IBM/CuAt
		CuAt.vc ---- 777 0 0 /tmp/rspc.tapeboot_IBM/CuAt.vc
		CuDep	---- 777 0 0 /tmp/rspc.tapeboot_IBM/CuDep
		CuDv	---- 777 0 0 /tmp/rspc.tapeboot_IBM/CuDv
		CuDvDr	---- 777 0 0 /tmp/rspc.tapeboot_IBM/CuDvDr
		CuVPD	---- 777 0 0 /tmp/rspc.tapeboot_IBM/CuVPD
		PdAt	---- 777 0 0 /tmp/rspc.tapeboot_IBM/PdAt
		PdAt.vc ---- 777 0 0 /tmp/rspc.tapeboot_IBM/PdAt.vc
		PdCn	---- 777 0 0 /tmp/rspc.tapeboot_IBM/PdCn
		PdDv	---- 777 0 0 /tmp/rspc.tapeboot_IBM/PdDv
		PdDv.vc ---- 777 0 0 /tmp/rspc.tapeboot_IBM/PdDv.vc
		$
	vg	d--- 777 0 0
		$
	$
lib	l--- 755 0 0 /usr/lib
mnt	d--- 755 0 0
	$
sbin	d--- 755 0 0
	rc.boot   ---- 777 0 0 /sbin/rc.boot
	$
tmp	d--- 777 0 0
	$
usr	d--- 755 0 0
	bin	d--- 755 0 0
		bsh  l--- 555 0 0 /usr/bin/ksh
		ksh  ---- 555 0 0 /usr/bin/ksh
		mkdir  ---- 555 0 0 /usr/bin/mkdir
		odmadd  ---- 777 0 0 /usr/bin/odmadd
		odmdelete  ---- 777 0 0 /usr/bin/odmdelete
		odmget  ---- 777 0 0 /usr/bin/odmget
		sh  l--- 555 0 0 /usr/bin/ksh
		tctl  ---- 777 0 0 /usr/bin/tctl
		$
	lib	d--- 755 0 0
		drivers	d--- 777 0 0
			ataide	---- 777 0 0 /usr/lib/drivers/ataide
			ataidepin	---- 777 0 0 /usr/lib/drivers/ataidepin
			bbldd		----  555  0 0   /usr/lib/drivers/bbldd
			bblddpin	----  555  0 0   /usr/lib/drivers/bblddpin
			hd_pin		----  550  0 0   /usr/lib/drivers/hd_pin
			hd_pin_bot	----  550  0 0   /usr/lib/drivers/hd_pin_bot
			idedisk		----  555  0 0   /usr/lib/drivers/idedisk
			idediskpin	----  555  0 0   /usr/lib/drivers/idediskpin
			isa	d--- 755 0 0
				fdisa	----  555  0 0   /usr/lib/drivers/isa/fdisa
				kbddd	----  555  0 0   /usr/lib/drivers/isa/kbddd
				rsdd_rspc	----  555  0 0   /usr/lib/drivers/isa/rsdd_rspc
				rspcios   ---- 777 0 0 /usr/lib/drivers/isa/rspcios
				$
			ldterm	----  555  0 0   /usr/lib/drivers/ldterm
			lft_loadpin	----  555  0 0   /usr/lib/drivers/lft_loadpin
			pci	d--- 755 0 0
				fgadd ----  555  0 0   /usr/lib/drivers/pci/fgadd
				fgadd_pin ----  555  0 0   /usr/lib/drivers/pci/fgadd_pin
				ggadd ----  555  0 0   /usr/lib/drivers/pci/ggadd
				ggadd_pin ----  555  0 0   /usr/lib/drivers/pci/ggadd_pin
				ggadd_video ----  555  0 0   /usr/lib/drivers/pci/ggadd_video
				hgadd ----  555  0 0   /usr/lib/drivers/pci/hgadd
				hgadd_pin ----  555  0 0   /usr/lib/drivers/pci/hgadd_pin
				igadd ----  555  0 0   /usr/lib/drivers/pci/igadd
				igadd_pin ----  555  0 0   /usr/lib/drivers/pci/igadd_pin
				ncr810dd ----  555  0 0   /usr/lib/drivers/pci/ncr810dd
				vcadd ----  555  0 0   /usr/lib/drivers/pci/vcadd
				vcadd_pin ----  555  0 0   /usr/lib/drivers/pci/vcadd_pin
				wfgdd ----  555  0 0   /usr/lib/drivers/pci/wfgdd
				wfgdd_pin ----  555  0 0   /usr/lib/drivers/pci/wfgdd_pin
				$
			pse	d--- 755 0 0
				pse ---- 777 0 0 /usr/lib/drivers/pse/pse
				$
			scdisk ----  555  0 0   /usr/lib/drivers/scdisk
			scdiskpin ----  555  0 0   /usr/lib/drivers/scdiskpin
			rsdd	----  644  0 0   /usr/lib/drivers/rsdd
			stydd	----  555  0 0   /usr/lib/drivers/stydd
			tape ----  555  0 0   /usr/lib/drivers/tape
			tapepin ----  555  0 0   /usr/lib/drivers/tapepin
			tioc	----  555  0 0   /usr/lib/drivers/tioc
			ttydbg	----  555  0 0   /usr/lib/drivers/ttydbg
			$
		libc.a ---- 555 0 0 /usr/ccs/lib/libc.a.min
		libcfg.a ---- 555 0 0 /tmp/libcfg.a
		libodm.a ---- 555 0 0 /tmp/libodm.a
		libs.a ---- 555 0 0 /usr/lib/libs.a.min
		methods d--- 777 0 0
			cfg_fga ----  500  0 0   /usr/lib/methods/cfg_fga
			cfg_gga ----  500  0 0   /usr/lib/methods/cfg_gga
			cfg_hga ----  500  0 0   /usr/lib/methods/cfg_hga
			cfg_iga ----  500  0 0   /usr/lib/methods/cfg_iga
			cfg_wfg ----  500  0 0   /usr/lib/methods/cfg_wfg
			cfgasync_rspc ----  500  0 0   /usr/lib/methods/cfgasync_rspc
			cfgataide ---- 777 0 0 /usr/lib/methods/cfgataide
			cfgbbl_pci ----  500  0 0   /usr/lib/methods/cfgbbl_pci
			cfgbus_pci ---- 777 0 0 /usr/lib/methods/cfgbus_pci
			cfgbus_isa ---- 777 0 0 /usr/lib/methods/cfgbus_isa
			cfgfda_isa ----  500  0 0   /usr/lib/methods/cfgfda_isa
			cfgfdd ----  500  0 0   /usr/lib/methods/cfgfdd
			cfgidedisk ----  500  0 0   /usr/lib/methods/cfgidedisk
			cfgkbd_std ----  500  0 0   /usr/lib/methods/cfgkbd_std
			cfgkm_isa ----  500  0 0   /usr/lib/methods/cfgkm_isa
			cfglft ----  500  0 0   /usr/lib/methods/cfglft
			cfglvdd ----  500  0 0   /usr/lib/methods/cfglvdd
			cfgncr_scsi ---- 777 0 0 /usr/lib/methods/cfgncr_scsi
			cfgscdisk ----  500  0 0   /usr/lib/methods/cfgscdisk
			cfgsctape ----  500  0 0   /usr/lib/methods/cfgsctape
			cfgsio_isa ---- 777 0 0 /usr/lib/methods/cfgsio_isa
			cfgsys_rspc ---- 777 0 0 /usr/lib/methods/cfgsys_rspc
			cfgtty ----  500  0 0   /usr/lib/methods/cfgtty
			isaload ----  500  0 0   /usr/lib/methods/isaload
			chggen ---- 777 0 0 /usr/lib/methods/chggen
			chggen_rspc ---- 777 0 0 /usr/lib/methods/chggen_rspc
			define ---- 777 0 0 /usr/lib/methods/define
			define_rspc ---- 777 0 0 /usr/lib/methods/define_rspc
			deflvm ----  500  0 0   /usr/lib/methods/deflvm
			defsys ---- 777 0 0 /usr/lib/methods/defsys
			instdbcln ---- 777 0 0 /usr/lib/methods/instdbcln
			showled ---- 777 0 0 /usr/lib/methods/showled
			startlft ---- 777 0 0 /usr/lib/methods/startlft
			starttty ---- 777 0 0 /usr/lib/methods/starttty
			ucfgdevice ---- 777 0 0 /usr/lib/methods/ucfgdevice
			$
		microcode d--- 777 0 0
			$
		nls	d--- 755 0 0
			loc	d--- 755 0 0
				C.lftkeymap ---- 777 0 0 /usr/lib/nls/loc/C.lftkeymap
				$
			$
		$
	lpp	d--- 755 0 0
		bosinst	d--- 755 0 0
			bi_main ---- 777 0 0 /usr/lpp/bosinst/bi_main
			$
		fonts	d--- 755 0 0
			Erg11.iso1.snf ---- 777 0 0 /usr/lpp/fonts/Erg11.iso1.snf
			Erg22.iso1.snf ---- 777 0 0 /usr/lpp/fonts/Erg22.iso1.snf
			$
		$
	sbin	d--- 755 0 0
		bootinfo ---- 777 0 0 /usr/sbin/bootinfo
		cfgmgr ---- 777 0 0 /usr/sbin/cfgmgr
		lsdev	---- 777 0 0 /usr/sbin/lsdev
		restbyname ---- 777 0 0 /usr/sbin/restbyname
		strload	---- 777 0 0 /usr/sbin/strload
		$
	$
$
