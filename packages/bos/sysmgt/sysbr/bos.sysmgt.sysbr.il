# @(#)74        1.10  src/packages/bos/sysmgt/sysbr/bos.sysmgt.sysbr.il, pkgsysbr, pkg411, 9435A411a 8/29/94 14:06:45
#
# COMPONENT_NAME: pkgsysbr
#
# FUNCTIONS:  inslist
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993,1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# -------------------------------------------------------------------------
#
# LPP_NAME: bos.sysmgt.sysbr
#
f 0 0 550       /etc/preserve.list mksysb
F 2 2 544	/usr/bin/mksysb mksysb
S 2 2 777	/usr/bin/savevg /usr/bin/mksysb mksysb
F 2 2 544	/usr/bin/mkszfile mksysb
S 2 2 777	/usr/bin/mkvgdata /usr/bin/mkszfile mksysb
F 2 2 544	/usr/bin/restvg mksysb
F 0 0 550       /usr/lpp/bosinst/BosMenus mksysb
F 0 0 550       /usr/lpp/bosinst/CheckSize mksysb
F 0 0 550       /usr/lpp/bosinst/Get_RVG_Disks mksysb
F 0 0 550       /usr/lpp/bosinst/adjfs mksysb
F 0 0 550       /usr/lpp/bosinst/bicfgsup mksysb
F 0 0 550       /usr/lpp/bosinst/bi_main mksysb
F 0 0 440       /usr/lpp/bosinst/bosinst.template mksysb
F 0 0 440       /usr/lpp/bosinst/image.template mksysb
D 0 0 755       /usr/lpp/bosinst/tape mksysb
F 0 0 644       /usr/lpp/bosinst/tape/tapefiles1 mksysb
F 2 2 555       /usr/sbin/mkinsttape mksysb
