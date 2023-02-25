#!/bin/ksh
# @(#)76 1.2  src/packages/bos/sysmgt/nim/master/usr/bos.sysmgt.nim.master.pre_d.sh, pkgnim, pkg411, GOLD410  6/29/94  16:28:56
#
#   COMPONENT_NAME: CMDNIM
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

type=$( /usr/bin/grep NIM_CONFIGURATION /etc/niminfo 2>/dev/null )
[[ "${type##*=}" = master ]] && exit 1
exit 0
