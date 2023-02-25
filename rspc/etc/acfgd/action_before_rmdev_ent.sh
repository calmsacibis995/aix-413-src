#!/bin/ksh
# @(#)14	1.1  src/rspc/etc/acfgd/action_before_rmdev_ent.sh, pcmciaent, rspc41J, 9509A_all 2/22/95 10:57:53
#
#   COMPONENT_NAME: PCMCIAENT
#
#   FUNCTIONS: Auto configuration script executed before the unconfiguration
#              method for ethernet runs on PCMCIA ethernet card removal.
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

devname=$1
ifname=en${devname#ent}
ifname2=et${devname#ent}

/usr/sbin/rmdev -l $ifname
/usr/sbin/rmdev -l $ifname2

# Even if the second rmdev command exit non 0 code,
# return exit code 0 to auto-config daemon
exit 0

