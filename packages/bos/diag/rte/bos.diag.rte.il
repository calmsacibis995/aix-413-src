# @(#)64 1.35 src/packages/bos/diag/rte/bos.diag.rte.il, pkgdiag, pkg41J, 9520B_all 5/19/95 13:11:21
#
# COMPONENT_NAME: PKGDIAG
#
# FUNCTIONS:  inslist
#
# ORIGINS: 27, 83
#
# (C) COPYRIGHT International Business Machines Corp. 1993, 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# LEVEL 1, 5 Years Bull Confidential Information
#
# -------------------------------------------------------------------------
#
# LPP_NAME: bos.diag.rte
#
d 0 0 755       /etc/lpp
d 0 0 755       /etc/lpp/diagnostics
d 0 0 755       /etc/lpp/diagnostics/data
v 0 0 644       /etc/lpp/diagnostics/data/fastdiag
D 2 2 755       /usr/include/diag
F 2 2 444       /usr/include/diag/atu.h
F 2 2 444       /usr/include/diag/bit_def.h
F 2 2 444       /usr/include/diag/da.h
F 2 2 444       /usr/include/diag/diag_exit.h
F 2 2 444       /usr/include/diag/diag_define.h
F 2 2 444       /usr/include/diag/diag.h
F 2 2 444       /usr/include/diag/diagodm.h
F 2 2 444       /usr/include/diag/diago.h
F 2 2 444       /usr/include/diag/tmdefs.h
F 2 2 444       /usr/include/diag/tm_input.h
F 2 2 444       /usr/include/diag/diag_errids.h
F 2 2 444       /usr/include/sys/diagex.h
F 0 0 444       /usr/lib/diagex.exp
V 0 0 444       /usr/lib/libdiag.a
F 0 0 500       /usr/lib/drivers/diagex
F 0 0 500       /usr/lib/methods/cfgdiag
F 0 0 500       /usr/lib/methods/ucfgdiag
D 0 0 755       /usr/lpp/diagnostics
F 0 0 444       /usr/lpp/diagnostics/CEREADME
D 0 0 755       /usr/lpp/diagnostics/bin
F 0 0 544       /usr/lpp/diagnostics/bin/dctrl
F 0 0 544       /usr/lpp/diagnostics/bin/diagela
F 0 0 544       /usr/lpp/diagnostics/bin/diagipl
F 0 0 544       /usr/lpp/diagnostics/bin/diagpt
F 0 0 544       /usr/lpp/diagnostics/bin/diagrpt
F 0 0 544       /usr/lpp/diagnostics/bin/diags
F 0 0 544       /usr/lpp/diagnostics/bin/diagdskpt
F 0 0 544       /usr/lpp/diagnostics/bin/get_diag_stanzas
F 0 0 544       /usr/lpp/diagnostics/bin/diagd
F 0 0 544       /usr/lpp/diagnostics/bin/udiagmon
F 0 0 444       /usr/lpp/diagnostics/bin/dskpt.dep
F 0 0 544       /usr/lpp/diagnostics/bin/dsense
F 0 0 444       /usr/lpp/diagnostics/bin/dsense.lib
F 0 0 544       /usr/lpp/diagnostics/bin/dutil
F 0 0 444       /usr/lpp/diagnostics/bin/labels.dsense
F 0 0 444       /usr/lpp/diagnostics/bin/rtypes.dsense
F 0 0 444       /usr/lpp/diagnostics/bin/arrays.dsense
F 0 0 544       /usr/lpp/diagnostics/bin/sysx
F 0 0 544       /usr/lpp/diagnostics/bin/u5081
F 0 0 544       /usr/lpp/diagnostics/bin/ucfgdev
F 0 0 544       /usr/lpp/diagnostics/bin/ucfgvpd
F 0 0 544       /usr/lpp/diagnostics/bin/udiagup
F 0 0 544       /usr/lpp/diagnostics/bin/udsense
F 0 0 544       /usr/lpp/diagnostics/bin/udtl
F 0 0 544       /usr/lpp/diagnostics/bin/ufd
F 0 0 544       /usr/lpp/diagnostics/bin/ulan
F 0 0 544       /usr/lpp/diagnostics/bin/uenet
F 0 0 544       /usr/lpp/diagnostics/bin/saluenet
F 0 0 544       /usr/lpp/diagnostics/bin/stiluenet
F 0 0 544       /usr/lpp/diagnostics/bin/umblist
F 0 0 544       /usr/lpp/diagnostics/bin/umcode
F 0 0 544       /usr/lpp/diagnostics/bin/umkdskt
F 0 0 544       /usr/lpp/diagnostics/bin/umlc
F 0 0 544       /usr/lpp/diagnostics/bin/uscsi
F 0 0 544       /usr/lpp/diagnostics/bin/uhint
F 0 0 544       /usr/lpp/diagnostics/bin/ush
F 0 0 544       /usr/lpp/diagnostics/bin/ump
F 0 0 544       /usr/lpp/diagnostics/bin/ubump
F 0 0 544       /usr/lpp/diagnostics/bin/ukey
F 0 0 544       /usr/lpp/diagnostics/bin/ugenucodesa
F 0 0 544       /usr/lpp/diagnostics/bin/ubackrest
F 0 0 544       /usr/lpp/diagnostics/bin/udiskmnt
F 0 0 544       /usr/lpp/diagnostics/bin/udmedit
F 0 0 544       /usr/lpp/diagnostics/bin/udisacfg
D 0 0 755       /usr/lpp/diagnostics/catalog
D 0 0 755       /usr/lpp/diagnostics/catalog/default
V 0 0 444       /usr/lpp/diagnostics/catalog/default/dcda.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/dctrl.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/diagrpt.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/diags.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/dutil.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/morps.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/sysx.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/u5081.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/ucfgdev.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/ucfgvpd.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/udiagup.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/udiagmon.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/udsense.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/udtl.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/uenet.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/ufd.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/ulan.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/umblist.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/umcode.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/umkdskt.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/umlc.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/uscsi.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/uhint.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/ump.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/ubump.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/ukey.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/ugenucodesa.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/ubackrest.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/udiskmnt.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/udmedit.cat
F 0 0 444       /usr/lpp/diagnostics/catalog/default/udisacfg.cat
F 0 0 544       /usr/lpp/diagnostics/catalog/default/sys.cat
F 0 0 544       /usr/lpp/diagnostics/catalog/default/diop.cat
F 0 0 544       /usr/lpp/diagnostics/catalog/default/memory.cat
F 0 0 544       /usr/lpp/diagnostics/catalog/default/fpp.cat
D 0 0 755       /usr/lpp/diagnostics/catalog/prime
D 0 0 755       /usr/lpp/diagnostics/lib
V 0 0 444       /usr/lpp/diagnostics/lib/libdiag.a
D 0 0 755       /usr/lpp/diagnostics/obj
F 0 0 755       /usr/lpp/diagnostics/obj/CDiagDev
F 0 0 755       /usr/lpp/diagnostics/obj/MenuGoal
F 0 0 755       /usr/lpp/diagnostics/obj/TMInput
F 0 0 755       /usr/lpp/diagnostics/obj/DAVars
F 0 0 755       /usr/lpp/diagnostics/obj/FRUB
F 0 0 755       /usr/lpp/diagnostics/obj/FRUs
D 0 0 755       /usr/lpp/diagnostics/da
F 0 0 544       /usr/lpp/diagnostics/da/dmorps
F 0 0 544       /usr/lpp/diagnostics/da/dfpp
F 0 0 544       /usr/lpp/diagnostics/da/dsys
F 0 0 544       /usr/lpp/diagnostics/da/dmptu
F 0 0 544       /usr/lpp/diagnostics/da/diop
F 0 0 544       /usr/lpp/diagnostics/da/diopsl
F 0 0 544       /usr/lpp/diagnostics/da/dmemory
F 2 2 100       /usr/sbin/feprom_update
F 2 2 100       /usr/sbin/mpcfg
F 2 2 100       /usr/sbin/keycfg
