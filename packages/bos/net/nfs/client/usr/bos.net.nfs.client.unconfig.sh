#!/bin/bsh
# @(#)09	1.1  src/packages/bos/net/nfs/client/usr/bos.net.nfs.client.unconfig.sh, pkgnfs, pkg411, GOLD410 6/30/94 20:41:38
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

if [ ! -f /usr/lib/drivers/nfs_kdes_full.ext ] ; then
	rm -f /usr/lib/drivers/nfs_kdes.ext
fi

exit 0
