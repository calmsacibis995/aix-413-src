#!/bin/ksh
# @(#)98 1.2  src/packages/ipx/rte/root/ipx.rte.config.sh, pkgnetware, pkg411, 9438B411a 9/15/94 09:56:23
#
#   COMPONENT_NAME: NETW_INSTALL
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1991,1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

lsitab "rcnetw" > /dev/null 2>&1
if [ $? -ne 0 ]    # The entry is not in the table.
then
	mkitab -i srcmstr "rcnetw:2:wait:/etc/rc.netware #start Netware"
	if [ $? -ne 0 ]
	then
		echo "Error adding rc.netware to inittab"
		exit 1
	fi
fi

# change mode on /etc/rc.netware.  If this was a migration
# this file will be preserved with the old 325 permissions
# which don't work for 411.  The file must have execute
# permission in 411.  

chmod 755 /etc/rc.netware

exit 0
