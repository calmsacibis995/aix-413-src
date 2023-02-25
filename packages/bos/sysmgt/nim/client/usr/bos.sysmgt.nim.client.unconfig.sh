#!/bin/ksh
# @(#)95	1.2  src/packages/bos/sysmgt/nim/client/usr/bos.sysmgt.nim.client.unconfig.sh, pkgnim, pkg411, GOLD410  3/24/94  18:37:09
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: 
#		/src/packages/bos/sysmgt/nim/client/bos.sysmgt.nim.client.unconfig.sh
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

# remove the nimclient /etc/inittab entry
/usr/sbin/rmitab nimclient 2>/dev/null

# remove the niminfo file
rm /etc/niminfo 2>/dev/null

exit 0
