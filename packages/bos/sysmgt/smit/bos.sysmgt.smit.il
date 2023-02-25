# @(#)98	1.16  src/packages/bos/sysmgt/smit/bos.sysmgt.smit.il, pkgsmit, pkg411, 9439A411a 9/27/94 11:36:28
#
# COMPONENT_NAME: pkgsmit
#
# FUNCTIONS:  inslist
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993.
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# -------------------------------------------------------------------------
#
# LPP_NAME: bos.sysmgt.smit.il
#
F 2 2 555	/usr/bin/smit smit
F 2 2 555	/usr/bin/smitty smit
F 2 2 555	/usr/sbin/piomisc_base smit
F 0 0 500	/usr/lib/instl/sm_inst smit
F 0 0 500	/usr/lib/instl/get_license_info smit

# Files needed for install assistant
D 0 0 755	/usr/lib/assist
F 0 0 500	/usr/lib/assist/calc_pgspace
F 0 0 500	/usr/lib/assist/set_pgspace
F 0 0 500	/usr/lib/assist/install_pkg
F 0 0 500	/usr/lib/assist/exit_assistant
F 0 0 500	/usr/lib/assist/check_cd
F 0 0 500	/usr/lib/assist/mount_cd
F 0 0 500	/usr/lib/assist/unmount_cd
F 0 0 500	/usr/lib/assist/assist_main
F 0 0 500	/usr/lib/assist/assist_migrate
F 0 0 500	/usr/lib/assist/sign_license
F 0 0 500	/usr/sbin/install_assist
D 0 0 755	/usr/lib/assist/add_files
F 0 0 500       /usr/lib/assist/add_files/sm_instdev.add
F 0 0 500       /usr/lib/assist/add_files/sm_isa.add
F 0 0 500       /usr/lib/assist/add_files/sm_migrate.add
F 0 0 500       /usr/lib/assist/add_files/sm_overwrite.add
F 0 0 500       /usr/lib/assist/add_files/sm_preinstall.add
F 0 0 500       /usr/lib/assist/add_files/sm_preserve.add
D 0 0 755	/usr/lib/assist/bitmaps
F 0 0 500	/usr/lib/assist/bitmaps/Toronto.bm
F 0 0 500	/usr/lib/assist/bitmaps/AIX.bm
