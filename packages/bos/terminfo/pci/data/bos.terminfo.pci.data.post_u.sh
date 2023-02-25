#!/bin/ksh
# @(#)02	1.1  src/packages/bos/terminfo/pci/data/bos.terminfo.pci.data.post_u.sh, pkgterm, pkg41J, 9515A_all 4/4/95 16:01:52
#
# COMPONENT_NAME: pkgterm
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1995
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
