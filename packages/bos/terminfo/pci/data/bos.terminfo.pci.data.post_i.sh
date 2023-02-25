#!/bin/ksh
#  @(#)42        1.2  src/packages/bos/terminfo/pci/data/bos.terminfo.pci.data.post_i.sh, pkgterm, pkg411, GOLD410 9/9/93 %U
#
# COMPONENT_NAME: pkgterm
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME: bos.terminfo.pci.data.post_i.sh
#
# FUNCTION:  Script called to run tic on the .ti files
#

export TERMINFO=/usr/share/lib/terminfo
/usr/bin/tic /usr/share/lib/terminfo/vt220-pc.ti
exit $?
