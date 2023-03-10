# @(#)10	1.4  src/bos/diag/boot/tapediag3A.dep.S, diagboot, bos41J, 9513A_all 3/27/95 %U
#
# COMPONENT_NAME: DIAGBOOT - TAPE DIAGNOSTIC PACKAGE
#
# FUNCTIONS: Diagnostic Tape 3A Content File
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
#	skyway
./usr/lib/methods/cfgsky
./usr/lib/drivers/skydd
./usr/lpp/diagnostics/da/dsky
./usr/lpp/diagnostics/da/dskym
./usr/lpp/diagnostics/catalog/default/dsky.cat
./usr/lpp/diagnostics/catalog/default/dskym.cat
#       sga
./usr/lpp/diagnostics/da/dsga
./usr/lpp/diagnostics/catalog/default/dsga.cat
./usr/lib/drivers/sgadd
./usr/lib/methods/cfgsga
./usr/lib/methods/cfg_sgabus
#       wga
./usr/lpp/diagnostics/da/dwga
./usr/lpp/diagnostics/catalog/default/dwga.cat
./usr/lib/drivers/wgadd_pin
./usr/lib/drivers/wgadd
./usr/lib/methods/cfg_wga
./usr/lib/methods/cfg_graphics
#	BBLUE
#./usr/lib/drivers/bbldd
#./usr/lib/drivers/bblddpin
#./usr/lib/methods/cfg_bbl
#./usr/lpp/diagnostics/da/dbblue
#./usr/lpp/diagnostics/catalog/default/dbblue.cat
# 	8f9a
#./usr/lib/drivers/nepdd_load
#./usr/lib/drivers/nepdd_loadpin
#./usr/lib/methods/cfg_nep
#./usr/lib/microcode/8f9a.00.00
#./usr/lpp/diagnostics/da/dnep
#./usr/lpp/diagnostics/catalog/default/dnep.cat
#	mag
#./usr/lib/methods/cfg_mag
#./usr/lib/drivers/magdd_load
#./usr/lib/drivers/magdd_loadpin
#./usr/lpp/diagnostics/da/dmag
#./usr/lpp/diagnostics/catalog/default/dmag.cat
