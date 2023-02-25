#!/bin/ksh
# @(#)85	1.3  src/packages/bos/sysmgt/nim/master/usr/bos.sysmgt.nim.master.config.sh, pkgnim, pkg411, GOLD410  6/2/94  18:07:23
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: 
#		/src/packages/bos/sysmgt/nim/client/bos.sysmgt.nim.master.config.sh
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

# if an /etc/niminfo exists, move it out of the way
[[ -f /etc/niminfo ]] && /usr/bin/cp /etc/niminfo /etc/niminfo.prev

# if an /.rhosts exists, move it out of the way
[[ -f /.rhosts ]] && /usr/bin/cp /.rhosts /.rhosts.prev

# save the nimclient inittab entry if it's there
/usr/bin/egrep "^nimclient:" /etc/inittab >/usr/lpp/bos.sysmgt/nim.client.itab

# now, remove it
/usr/sbin/rmitab nimclient 2>/dev/null

exit 0
