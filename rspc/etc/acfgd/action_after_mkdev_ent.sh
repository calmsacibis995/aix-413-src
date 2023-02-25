#!/bin/ksh
# @(#)13	1.1  src/rspc/etc/acfgd/action_after_mkdev_ent.sh, pcmciaent, rspc41J, 9509A_all 2/22/95 10:57:52
#
#   COMPONENT_NAME: PCMCIAENT
#
#   FUNCTIONS: Auto configuration script executed after the configuration
#              method for ethernet runs on PCMCIA ethernet card insertion.
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

/usr/lib/methods/definet >/dev/null 2>&1

/usr/sbin/chdev -l $ifname -a state=up >/dev/null 2>&1
exitcode1=$?
/usr/sbin/chdev -l $ifname2 -a state=up >/dev/null 2>&1
exitcode2=$?

if [[ $exitcode1 -ne 0 && $exitcode2 -ne 0 ]] ; then
 /usr/sbin/mkdev -c if -s EN -t en -w $ifname >/dev/null 2>&1
 /usr/sbin/mkdev -c if -s EN -t ie3 -w $ifname2 >/dev/null 2>&1
fi

exitcode=$?

if [[ $exitcode -eq 0 ]] ; then
 /usr/sbin/mkdev -l inet0
 exitcode=$?
fi

exit $exitcode
