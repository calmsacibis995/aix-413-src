#if 0
# @(#)51  1.18  src/packages/bos/bosboot/cd.proto.S, bosboot, bos41J, 9517B_all 4/28/95 11:39:33
#
#   COMPONENT_NAME: BOSBOOT
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#endif
/bootrec 6144 0
d--- 755 0 0
bin     l--- 777 0 0 /usr/bin
dev     d--- 755 0 0
	bus0	  c--- 640 0 0 3 0
	console   c--- 622 0 0 4 0
	error	  c--- 622 0 0 6 0
	errorctl  c--- 600 0 0 6 1
	kmem 	  c--- 440 0 0 2 1
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
	init       ---- 777 0 0 /usr/lib/boot/ssh
	methods l--- 777 0 0 /usr/lib/methods
	microcode l--- 777 0 0 /usr/lib/microcode
	objrepos d--- 777 0 0
		Config_Rules	---- 777 0 0 /tmp/cdboot_IBM/Config_Rules
		CuAt	---- 777 0 0 /tmp/cdboot_IBM/CuAt
		CuAt.vc	---- 777 0 0 /tmp/cdboot_IBM/CuAt.vc
		CuDep	---- 777 0 0 /tmp/cdboot_IBM/CuDep
		CuDv	---- 777 0 0 /tmp/cdboot_IBM/CuDv
		CuDvDr	---- 777 0 0 /tmp/cdboot_IBM/CuDvDr
		CuVPD	---- 777 0 0 /tmp/cdboot_IBM/CuVPD
		PdAt	---- 777 0 0 /tmp/cdboot_IBM/PdAt
		PdAt.vc	---- 777 0 0 /tmp/cdboot_IBM/PdAt.vc
		PdCn	---- 777 0 0 /tmp/cdboot_IBM/PdCn
		PdDv	---- 777 0 0 /tmp/cdboot_IBM/PdDv
		PdDv.vc	---- 777 0 0 /tmp/cdboot_IBM/PdDv.vc
		$
        vfs      ---- 555 0 0 /etc/vfs
	vg	 d--- 777 0 0
		$
	$
lib	l--- 755 0 0 /usr/lib
mnt	d--- 755 0 0
	$
SPOT	d--- 755 0 0
	$
sbin	d--- 755 0 0
	rc.boot   ---- 777 0 0 /sbin/rc.boot
	$
tmp	d--- 777 0 0
	$
usr	d--- 755 0 0
	bin	d--- 755 0 0
		bsh      l--- 555 0 0 /usr/bin/ksh
		ksh      ---- 555 0 0 /usr/bin/ksh
		sh      l--- 555 0 0 /usr/bin/ksh
		$
	lib	d--- 755 0 0
		drivers	d--- 777 0 0
			ascsidd     ---- 777 0 0 /usr/lib/drivers/ascsidd
			ascsiddpin  ---- 777 0 0 /usr/lib/drivers/ascsiddpin
        		cfs.ext      ---- 555 0 0 /usr/lib/drivers/cfs.ext
			hscsidd   ---- 777 0 0 /usr/lib/drivers/hscsidd
			pscsidd   ---- 777 0 0 /usr/lib/drivers/pscsidd
			pscsiddpin   ---- 777 0 0 /usr/lib/drivers/pscsiddpin
			pscsi720dd   ---- 777 0 0 /usr/lib/drivers/pscsi720dd
			pscsi720ddpin ---- 777 0 0 /usr/lib/drivers/pscsi720ddpin
			scdisk      ---- 777 0 0 /usr/lib/drivers/scdisk
			scdiskpin   ---- 777 0 0 /usr/lib/drivers/scdiskpin
			vscsidd     ---- 777 0 0 /usr/lib/drivers/vscsidd
			vscsiddpin  ---- 777 0 0 /usr/lib/drivers/vscsiddpin
			$
		libc.a	 ---- 555 0 0 /usr/ccs/lib/libc.a.min
		libcfg.a ---- 555 0 0 /tmp/libcfg.a
		libodm.a ---- 555 0 0 /tmp/libodm.a
		libs.a   ---- 555 0 0 /usr/lib/libs.a.min
		methods d--- 777 0 0
			cfgascsi    ---- 777 0 0 /usr/lib/methods/cfgascsi
			cfgbus   ---- 777 0 0 /usr/lib/methods/cfgbus
			cfghscsi  ---- 777 0 0 /usr/lib/methods/cfghscsi
			cfgpscsi  ---- 777 0 0 /usr/lib/methods/cfgpscsi
			cfgsccd ---- 777 0 0 /usr/lib/methods/cfgsccd
			cfgsio   ---- 777 0 0 /usr/lib/methods/cfgsio
			cfgsys   ---- 777 0 0 /usr/lib/methods/cfgsys
			cfgsys_p ---- 777 0 0 /usr/lib/methods/cfgsys_p
			cfgvscsi    ---- 777 0 0 /usr/lib/methods/cfgvscsi
			chggen  ---- 777 0 0 /usr/lib/methods/chggen
			define   ---- 777 0 0 /usr/lib/methods/define
			defsys   ---- 777 0 0 /usr/lib/methods/defsys
			showled	---- 777 0 0 /usr/lib/methods/showled
			ucfgdevice  ---- 777 0 0 /usr/lib/methods/ucfgdevice
			$
		microcode d--- 777 0 0
			$
		$
	sbin	d--- 755 0 0
		bootinfo   ---- 777 0 0 /usr/sbin/bootinfo
		cfgmgr     ---- 777 0 0 /usr/sbin/cfgmgr
        	mount      ---- 555 0 0 /usr/sbin/mount
		$
	$
$
