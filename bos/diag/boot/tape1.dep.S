# @(#)03	1.28  src/bos/diag/boot/tape1.dep.S, diagboot, bos41J, 9525A_all 6/16/95 16:12:08
#
#
# COMPONENT_NAME: DIAGBOOT
#
# FUNCTIONS: Diagnostic Tape 1 Contents File
#	     This file has all the methods and drivers needed
#	     to configure tty, lft, graphics adapters, keyboard
#	     mouse, tablet and base system files.
#	     
#		
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1994, 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
########################################################################
./usr/lib/drivers/diagex
./usr/sbin/mkdev
./usr/sbin/rmdev
./usr/bin/compress
./usr/lib/drivers/scarray
./usr/lpp/diagnostics/bin/dctrl
./usr/lpp/diagnostics/bin/diags
./usr/lpp/diagnostics/bin/dutil
./usr/lpp/diagnostics/bin/diagpt
./usr/lpp/diagnostics/da/dmemory
./usr/lpp/diagnostics/da/dsys
./usr/lpp/diagnostics/da/diop
./usr/lpp/diagnostics/da/diopsl
./usr/lpp/diagnostics/da/dfpp
./usr/lpp/diagnostics/catalog/default/dcda.cat
./usr/lpp/diagnostics/catalog/default/diags.cat
./usr/lpp/diagnostics/catalog/default/dctrl.cat
./usr/lpp/diagnostics/catalog/default/dutil.cat
./usr/lpp/diagnostics/catalog/default/memory.cat
./usr/lpp/diagnostics/catalog/default/sys.cat
./usr/lpp/diagnostics/catalog/default/fpp.cat
./usr/lpp/diagnostics/catalog/default/diop.cat
./usr/lpp/diagnostics/catalog/default/udiskmnt.cat
./usr/lpp/diagnostics/catalog/default/ubackrest.cat
./usr/lpp/diagnostics/catalog/default/ugenucodesa.cat
./usr/lpp/diagnostics/catalog/default/u7135.cat
./usr/lib/methods/graphdev.cat
#	tty
./usr/lib/drivers/rsdd
./usr/sbin/strload
./usr/lib/drivers/pse/pse
./usr/lib/drivers/tioc
./usr/lib/drivers/ldterm
./usr/lib/drivers/liondd
#       lft
./usr/lib/methods/cfglft
./usr/lib/methods/startlft
./usr/lib/drivers/lft_loadpin
#	rcm
./usr/lib/methods/startrcm
./usr/lib/methods/cfgrcm
./usr/lib/drivers/rcm_load
./usr/lib/drivers/rcm_loadpin
#	kts 
./usr/lib/drivers/tabletdd
./usr/lib/drivers/kbddd
./usr/lib/drivers/ktsdd
./usr/lib/methods/cfgkma
./usr/lib/methods/cfgktsm
./usr/lib/methods/cfgkbd_std
./usr/lib/methods/cfgmouse_std
./usr/lib/methods/cfgtablet_std
./usr/lib/drivers/mousedd
#	skyway
./usr/lib/methods/cfgsky
./usr/lib/drivers/skydd
./usr/lpp/diagnostics/da/dsky
./usr/lpp/diagnostics/da/dskym
#       sga
./usr/lib/drivers/sgadd
./usr/lib/methods/cfgsga
./usr/lib/methods/cfg_sgabus
./usr/lpp/diagnostics/da/dsga
#       wga
./usr/lib/drivers/wgadd_pin
./usr/lib/drivers/wgadd
./usr/lib/methods/cfg_wga
./usr/lib/methods/cfg_graphics
./usr/lpp/diagnostics/da/dwga
#	8f9a
#./usr/lib/drivers/nepdd_load
#./usr/lib/drivers/nepdd_loadpin
#./usr/lib/methods/cfg_nep
#./usr/lib/microcode/8f9a.00.00
#./usr/lpp/diagnostics/da/dnep
#	ped
./usr/lib/methods/cfg_mid
./usr/lib/drivers/middd_load
./usr/lib/drivers/middd_loadpin
./usr/lib/microcode/8ee3xb.01.01
./usr/lib/microcode/8ee3nb.01.01
./usr/lib/microcode/8ee3p4d1.00.01
./usr/lib/microcode/8ee3p4d2.00.01
./usr/lib/microcode/8ee3p4d3.00.01
./usr/lib/microcode/8ee3p4d4.00.01
./usr/lib/microcode/8ee3p4d5.00.01
./usr/lpp/diagnostics/da/dped
#	lega
./usr/lib/microcode/8ee3lb.00.01
./usr/lib/microcode/8ee3l2b.00.01
./usr/lib/microcode/8ee3ld1.00.04
./usr/lib/microcode/8ee3ld2.00.04
./usr/lib/microcode/8ee3ld3.00.04
./usr/lib/microcode/8ee3ld4.00.04
./usr/lib/microcode/8ee3ld5.00.04
./usr/lpp/diagnostics/da/dlega
#       baby blue
#./usr/lib/drivers/bbldd
#./usr/lib/drivers/bblddpin
#./usr/lib/methods/cfg_bbl
#./usr/lpp/diagnostics/da/dbblue
#	gemini
./usr/lib/methods/cfggem
./usr/lib/drivers/gemdd
./usr/lib/microcode/8ffdgu.00.04
./usr/lib/microcode/8ffdgt.00.04
./usr/lib/microcode/8ffdgv.00.04
./usr/lib/microcode/8ffdvp.00.03
./usr/lib/microcode/8ffdc2.00.04
./usr/lib/microcode/8ffdsh.00.04
./usr/lib/microcode/8ffddc25.00.02
./usr/lib/microcode/8ffddbif.00.01
./usr/lib/microcode/8ffddgcp.00.02
./usr/lpp/diagnostics/da/dgraph
#	Ruby
./usr/lib/methods/cfgrby
./usr/lib/drivers/rbydd
./usr/lib/microcode/8fbccp.00.00
./usr/lib/microcode/8fbcap.00.00
./usr/lib/microcode/8fbccp1.00.00
./usr/lib/microcode/8fbcap1.00.00
./usr/lib/microcode/8fbclogp.00.00
./usr/lib/microcode/8fbclogp1.00.00
./usr/lib/microcode/8fbchigp.00.00
./usr/lib/microcode/8fbchigp1.00.00
./usr/lpp/diagnostics/da/druby
#	mag
#./usr/lib/methods/cfg_mag
#./usr/lib/drivers/magdd_load
#./usr/lib/drivers/magdd_loadpin
#./usr/lpp/diagnostics/da/dmag
#       system console
./usr/lib/methods/cfgcon
#       async
./usr/lib/methods/rsddload
./usr/lib/methods/cxload
./usr/lib/methods/cfgasync
./usr/lib/methods/ucfgasync
./usr/lib/methods/cfgtty
./usr/lib/methods/chgtty
./usr/lib/methods/ucfgtty
./usr/lib/methods/starttty
./usr/lpp/diagnostics/da/dasync
#	kts
./usr/lpp/diagnostics/da/dkeytaba
./usr/lpp/diagnostics/da/dkbd
./usr/lpp/diagnostics/da/dkbdsal
./usr/lpp/diagnostics/da/dkmta
./usr/lpp/diagnostics/slih/kbd_slih
#   	128 sync stuff
./usr/lib/methods/cfgsync
./usr/lib/methods/ucfgsync
./usr/lib/methods/cfgcxma
./usr/lib/methods/ucfgcxma
./usr/lib/methods/chgcxma
./usr/lib/drivers/cxmadd
./usr/lbin/tty/stty-cxma
./usr/lib/microcode/cxbios.02.09
./usr/lib/microcode/cxfep.02.09
#	kazusa
./usr/lib/methods/cfgbadisk
./usr/lib/drivers/badisk
./usr/lib/drivers/badiskpin
#	cdrom
./usr/lib/methods/cfgsccd
#	scsi adapter
./usr/lib/methods/cfghscsi
./usr/lib/methods/cfgpscsi
./usr/lib/methods/cfgascsi
./usr/lib/methods/cfgvscsi
#	disks
./usr/lib/methods/cfgscdisk
./usr/lib/drivers/scdisk
./usr/lib/drivers/scdiskpin
#	serial disks
./usr/lib/methods/cfgserdasda
./usr/lib/methods/cfgserdasdc
./usr/lib/methods/cfgserdasdd
./usr/lib/methods/sdd
./usr/lib/drivers/sd
./usr/lib/drivers/sdpin
./usr/lib/microcode/8f78.00.16
#	disk array
./usr/lib/drivers/scarraypin
./usr/lib/methods/cfgscarray
./usr/lib/methods/cfgdac
./usr/lib/methods/cfgdar
./usr/lib/methods/chgdar
./usr/lib/methods/defscarray
# Do not include unconfig method to
# allow controllers to be available at
# all time.
#./usr/lib/methods/ucfgdac
./usr/lib/methods/ucfgdar
./usr/lib/methods/darcfgrule
#	ssa
#./usr/lib/methods/cfgssar
#./usr/lib/methods/cfgssadisk
#./usr/lib/methods/cfgssapdisk
#./usr/lib/methods/cfgssa
#./usr/lib/methods/defssadev
#./usr/lib/methods/defssar
#./usr/lib/methods/udefssadev
#./usr/lib/drivers/ssa
#./usr/lib/drivers/ssapin
#./usr/lib/drivers/ssadisk
#./usr/lib/drivers/ssadiskpin
#./usr/lib/nls/msg/en_US/ssa.cat
#./usr/lib/nls/msg/en_US/ssadisk.cat
#./usr/lib/nls/msg/en_US/SSAaids.cat
#	enet
./usr/lib/asw/geteibid.com
./usr/lib/methods/cfgent
./usr/lib/methods/ucfgent
./usr/lib/drivers/entdd
./usr/lib/methods/cfgdmxeth
./usr/lib/drivers/eth_demux
#	integrated Ethernet
./usr/lib/methods/cfgeth
./usr/lib/drivers/ethdd
#	mpqp
./usr/lib/methods/cfgmpqp
./usr/lib/methods/cfgmpqd
./usr/lib/methods/defmpqd
./usr/lib/drivers/mpqpdd
./usr/lib/asw/mpqpasw
#	token ring
./usr/lib/methods/cfgtok
./usr/lib/methods/ucfgtok
./usr/lib/methods/cfgdmxtok
./usr/lib/drivers/tok_demux
./usr/lib/drivers/tokdd
./usr/lib/microcode/8fc8m.00.02
./usr/lib/microcode/8fc8p.00.01
#	mps
./usr/lib/methods/cfgmps
./usr/lib/drivers/mpsdd
#	mpa
./usr/lib/methods/cfgmpaa
./usr/lib/methods/cfgmpa
./usr/lib/drivers/mpadd
#	x25
./usr/lib/methods/cfgampx
./usr/lib/methods/cfgapm
./usr/lib/methods/chggen
./usr/lib/microcode/f0efd.00.02
#	Artic
./usr/lib/microcode/708fd.00.00
#	fddi
./usr/lib/methods/cfgfddi
./usr/lib/methods/ucfgfddi
./usr/lib/drivers/fddidd
./usr/lib/methods/cfgdmxfddi
./usr/lib/drivers/fddi_demux
./usr/lib/microcode/8ef4m.02.04
#	370pc
./usr/lib/methods/cfgcat
./usr/lib/drivers/catdd
./usr/lib/microcode/fe92d.00.01
./usr/lib/microcode/fe92.00.03
./usr/lib/microcode/cu3088b.00.00
./usr/lib/microcode/cu3088e.00.00
./usr/lib/microcode/cu3172b.00.00
#	3270
./usr/lib/methods/cfg3270
./usr/lib/drivers/c327dd
#	psla/msla
./usr/lib/methods/cfgpsla
./usr/lib/methods/cfgmsla
./usr/lib/methods/cfghia
./usr/lib/drivers/psladd
./usr/lib/drivers/hiadd
./usr/lib/microcode/8787S.00.01
./usr/lib/microcode/8787d50.00.01
./usr/lib/microcode/8787dpl2.00.01
./usr/lib/microcode/8787dpl3.00.01
./usr/lib/microcode/8787dwrp.00.01
./usr/lib/microcode/8787dmod.00.01
./usr/lib/microcode/8787dp.00.01
./usr/lib/microcode/8787dd1.00.01
./usr/lib/microcode/8787dd2.00.01
#	Optics 2 port
./usr/lib/methods/cfgsol
./usr/lib/drivers/soldd
./usr/lib/drivers/solddpin
#	gio, dials, lpfks
./usr/lib/methods/cfggio
./usr/lib/methods/cfggiodev
./usr/lib/drivers/giodd
./usr/lib/methods/cfgsgio
./usr/lib/methods/startsgio
./usr/lib/drivers/sgiodd
#	parallel
./usr/lib/methods/cfgppa
./usr/lib/methods/cfgpp
./usr/lib/drivers/ppdd
./usr/lib/drivers/ppddpin
