# @(#)93	1.5  src/bos/diag/boot/graphics1.dep.S, diagboot, bos41J, 9513A_all 3/27/95 15:15:40
#
#
# COMPONENT_NAME: DIAGBOOT
#
# FUNCTIONS: Config stuff
#	     This file has all the methods and drivers needed
#	     to configure graphics adapters. This file will be passed
#	     to restbyname to only let a certain number of files to
#	     be restored into the RAM file system.
#		
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
########################################################################
#	color and gray scale display adapters
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
./usr/lpp/diagnostics/da/dwga
./usr/lib/methods/cfg_graphics
#	mag
#./usr/lib/drivers/magdd_load
#./usr/lib/drivers/magdd_loadpin
#./usr/lib/methods/cfg_mag
#./usr/lpp/diagnostics/da/dmag
#	4006 & 4007 and efc6
#./usr/lib/drivers/bbldd
#./usr/lib/drivers/bblddpin
#./usr/lib/methods/cfg_bbl
#./usr/lpp/diagnostics/da/dbblue
#	8f9a
#./usr/lib/drivers/nepdd_load
#./usr/lib/drivers/nepdd_loadpin
#./usr/lib/methods/cfg_nep
#./usr/lib/microcode/8f9a.00.00
#./usr/lpp/diagnostics/da/dnep
#	Gt4
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
./usr/lib/microcode/8ee3lb.00.01
./usr/lib/microcode/8ee3l2b.00.01
./usr/lib/microcode/8ee3ld1.00.04
./usr/lib/microcode/8ee3ld2.00.04
./usr/lib/microcode/8ee3ld3.00.04
./usr/lib/microcode/8ee3ld4.00.04
./usr/lib/microcode/8ee3ld5.00.04
./usr/lpp/diagnostics/da/dlega
