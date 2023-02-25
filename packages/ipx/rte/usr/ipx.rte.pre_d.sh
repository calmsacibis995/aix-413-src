#!/bin/ksh
# @(#)80 1.1  src/packages/ipx/rte/usr/ipx.rte.pre_d.sh, pkgnetware, pkg411, 9439B411a 9/27/94 14:34:40
# COMPONENT_NAME: netw_install
#
# FUNCTIONS: none
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1990, 1991
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# NAME:		src/packages/ipx/rte/usr/ipx.rte.pre_d.sh
#

# stop npsd and sapd if they are running

pid="`cat /etc/netware/npsd.pid 2> /dev/null`"
if [ "$pid" -ne 0 ]
then
   echo
   echo "Shutting down npsd and sapd..."
   echo
   /usr/lpp/netware/bin/stopnps

	# wait and then force the processes to die

	sleep 10

	kill -9 `ps -e | grep " sapd" | cut -c0-6` 2> /dev/null
	kill -9 `ps -e | grep " npsd" | cut -c0-6` 2> /dev/null
	kill -9 `ps -e | grep " nvts" | cut -c0-6` 2> /dev/null
fi

exit 0
