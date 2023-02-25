#!/bin/ksh
# @(#)50        1.2  src/packages/bos/terminfo/ansi/data/bos.terminfo.ansi.data.post_i.sh, pkgterm, pkg411, GOLD410 9/9/93 19:31:17
#
# COMPONENT_NAME: pkgterm
#
# FUNCTIONS: config
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME: bos.terminfo.ansi.data.config.sh
#
# FUNCTION:  Script called to run tic on the .ti files
#

export TERMINFO=/usr/share/lib/terminfo
tic /usr/share/lib/terminfo/ansi.ti
exit $?
