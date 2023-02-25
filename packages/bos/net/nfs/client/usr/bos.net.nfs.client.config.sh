#!/bin/bsh
# @(#)08	1.1  src/packages/bos/net/nfs/client/usr/bos.net.nfs.client.config.sh, pkgnfs, pkg411, GOLD410 6/30/94 20:41:31
#
# COMPONENT_NAME: pkgnfs
#
# FUNCTIONS: none
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
 
if [ ! -f /usr/lib/drivers/nfs_kdes.ext ] ; then
	ln -s /usr/lib/drivers/nfs_kdes_null.ext /usr/lib/drivers/nfs_kdes.ext
	if [ $? -ne 0 ]
	then
		exit 1
	fi
fi

exit 0
