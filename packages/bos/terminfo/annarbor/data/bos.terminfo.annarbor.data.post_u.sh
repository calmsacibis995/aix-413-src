#!/bin/ksh
# @(#)75	1.1  src/packages/bos/terminfo/annarbor/data/bos.terminfo.annarbor.data.post_u.sh, pkgterm, pkg41J, 9515A_all 4/4/95 15:59:02
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
# NAME: bos.terminfo.annarbor.data.config.sh
#
# FUNCTION:  Script called to run tic on the .ti files
#

export TERMINFO=/usr/share/lib/terminfo
tic /usr/share/lib/terminfo/annarbor.ti
exit $?
