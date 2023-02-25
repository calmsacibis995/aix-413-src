#!/bin/ksh
# @(#)55	1.1  src/rspc/usr/sbin/acfgd/lacfgd.sh, pcmciaker, rspc41J, 9512A_all 3/16/95 17:12:09
#
#   COMPONENT_NAME: PCMCIAKER
#
#   FUNCTIONS: Shell script to run auto-config daemon
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

ndevs=`/usr/sbin/lsdev -C -c bus -t pcmcia -S available -r name | wc -l`

if [[ $ndevs -ge 1 ]]
then
  /usr/sbin/acfgd >/dev/console 2>&1
fi

