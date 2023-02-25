# @(#)54        1.8  src/packages/bos/powermgt/rte/bos.powermgt.rte.il, pkgpwrmgt, pkg41J, 9520A_all 5/15/95 20:01:31
#
# COMPONENT_NAME: pkgpwrmgt
#
# FUNCTIONS:  inslist
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
# -------------------------------------------------------------------------
#
# LPP_NAME: bos.powermgt.rte
#
F 0 0 500	/usr/lib/methods/startpm
F 0 0 555	/usr/lib/drivers/pmmi
F 0 0 555	/usr/lib/drivers/pmmipin
F 0 0 555	/usr/lib/drivers/pmwakeup
F 0 0 755	/usr/bin/pmctrl
F 0 0 755	/usr/bin/battery
F 0 0 755	/usr/bin/pmd
F 0 0 755	/usr/bin/mkpmhlv
F 0 0 755	/usr/bin/checkpmc
V 2 2 555	/usr/lib/libpm.a
v 0 0 664	/etc/objrepos/pmmi_data
F 0 0 500	/usr/lib/boot/disable_hibernation
H 0 0 500	/usr/lib/boot/enable_hibernation /usr/lib/boot/disable_hibernation
