#!/bin/ksh
# @(#)91	1.1  src/packages/bos/terminfo/hardcopy/data/bos.terminfo.hardcopy.data.post_u.sh, pkgterm, pkg41J, 9515A_all 4/4/95 16:00:27
#
# COMPONENT_NAME: pkgterm
#
# FUNCTIONS: config
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME: bos.terminfo.hardcopy.data.config.sh
#
# FUNCTION:  Script called to run tic on the .ti files
#

export TERMINFO=/usr/share/lib/terminfo
tic /usr/share/lib/terminfo/hardcopy.ti
exit $?
