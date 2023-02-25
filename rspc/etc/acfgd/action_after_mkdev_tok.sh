#!/bin/ksh
# @(#)15	1.1  src/rspc/etc/acfgd/action_after_mkdev_tok.sh, pcmciatok, rspc41J, 9509A_all 2/22/95 10:58:19
#
#   COMPONENT_NAME: PCMCIATOK
#
#   FUNCTIONS: Auto configuration script executed after the configuration
#              method for tokenring runs on PCMCIA tokenring card insertion.
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

/usr/lib/methods/definet > /dev/null 2>&1

/usr/sbin/chdev -l $ifname -a state=up >/dev/null 2>&1

if [[ $? -ne 0 ]] ; then
 /usr/sbin/mkdev -c if -s TR -t tr -w $ifname >/dev/null 2>&1
fi

exitcode=$?

if [[ $exitcode -eq 0 ]] ; then
 /usr/sbin/mkdev -l inet0
 exitcode=$?
fi 

exit $exitcode
