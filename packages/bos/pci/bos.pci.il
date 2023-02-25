# @(#)80	1.3  src/packages/bos/pci/bos.pci.il, pkgpci, pkg411, GOLD410 1/28/94 15:51:10
#
# COMPONENT_NAME:  pkgpci
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
# LPP_NAME: bos.pci
#
f 0 0 755	/etc/rc.pci dos-pci
F 0 0 755	/usr/bin/charconv dos-pci
S 0 0 777	/usr/bin/aix2dos /usr/bin/charconv dos-pci
S 0 0 777	/usr/bin/dos2aix /usr/bin/charconv dos-pci
D 0 0 755	/usr/pci doc-pci
D 0 0 755	/usr/pci/bin doc-pci
F 0 0 755	/usr/pci/bin/environ dos-pci
F 0 0 755	/usr/pci/bin/errlogger dos-pci
F 0 0 755	/usr/pci/bin/lmfmsg dos-pci
F 0 0 755	/usr/pci/bin/pciconsvr.ip dos-pci
F 0 0 755	/usr/pci/bin/pcidebug dos-pci
F 0 0 755	/usr/pci/bin/pcidosout.232 dos-pci
F 0 0 755	/usr/pci/bin/pcidosout.ip dos-pci
F 0 0 755	/usr/pci/bin/pcidossvr.232 dos-pci
F 0 0 755	/usr/pci/bin/pcidossvr.ip dos-pci
F 0 0 755	/usr/pci/bin/pcimapsvr.ip dos-pci
F 0 0 755	/usr/pci/bin/pciprint dos-pci
F 0 0 755	/usr/pci/bin/pcistart dos-pci
F 0 0 755	/usr/pci/bin/pcistop dos-pci
F 0 0 755	/usr/pci/bin/rlockshm dos-pci
D 0 0 755	/usr/pci/dosmsg doc-pci
F 0 0 644	/usr/pci/dosmsg/C.lmf dos-pci
S 0 0 777	/usr/pci/dosmsg/En_US.lmf /usr/pci/dosmsg/C.lmf dos-pci
D 0 0 755	/usr/pci/lib doc-pci
F 0 0 644	/usr/pci/lib/8859.lcs dos-pci
F 0 0 644	/usr/pci/lib/pc437.lcs dos-pci
F 0 0 644	/usr/pci/lib/pc850.lcs dos-pci
F 0 0 644	/usr/pci/lib/pc860.lcs dos-pci
F 0 0 644	/usr/pci/lib/pc863.lcs dos-pci
F 0 0 644	/usr/pci/lib/pc865.lcs dos-pci
F 0 0 755	/usr/pci/pcidossvr.232 dos-pci
F 0 0 644	/usr/pci/pciptys dos-pci
D 0 0 777	/var/spool/pcilog doc-pci
