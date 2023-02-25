#!/bin/ksh
# @(#)16	1.1  src/rspc/etc/acfgd/action_before_rmdev_tok.sh, pcmciatok, rspc41J, 9509A_all 2/22/95 10:58:22
#
#   COMPONENT_NAME: PCMCIATOK
#
#   FUNCTIONS: Auto configuration script executed before the unconfiguration
#              method for tokenring runs on PCMCIA tokenring card removal.
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
ifname=tr${devname#tok}

/usr/sbin/rmdev -l $ifname

# Even if the rmdev command exit with non 0 code,
# return exit code 0 to auto-config daemon
exit 0

