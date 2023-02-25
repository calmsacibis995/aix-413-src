#!/bin/bsh
# @(#)06	1.1  src/packages/des/usr/des.unconfig.sh, pkgdes, pkg411, GOLD410 6/30/94 20:40:37
#
# COMPONENT_NAME: pkgdes
#
# FUNCTIONS: none
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

rm -f /usr/lib/drivers/nfs_kdes.ext

if [ -f /usr/lib/drivers/nfs_kdes_null.ext ] ; then
	ln -s /usr/lib/drivers/nfs_kdes_null.ext /usr/lib/drivers/nfs_kdes.ext
	if [ $? -ne 0 ]
	then
		exit 1
	fi
fi

exit 0
