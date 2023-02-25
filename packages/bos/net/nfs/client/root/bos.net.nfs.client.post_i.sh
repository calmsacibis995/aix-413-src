#!/bin/ksh
# @(#)23        1.1  src/packages/bos/net/nfs/client/root/bos.net.nfs.client.post_i.sh, pkgnfs, pkg411, 9438C411a 9/23/94 11:03:31
#
# COMPONENT_NAME: pkgnis
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
 
#
# NAME:  root/bos.net.nfs.client.post_i
#
# FUNCTION: script called from instal
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
#   
# This will migrate the nfs user config files if we are
# at 4.1 or higher.

FILESET=bos.net.nfs.client

if [ -n "$MIGSAVE" -a "$INSTALLED_LIST" ]; then
	echo $INSTALLED_LIST | grep -q $FILESET
	if [ $? = 0 ]; then
	/usr/bin/ed -s $FILESET.cfgfiles << END > /dev/null
1,\$s/user_merge/preserve/g
w
q
END
	fi 
fi
exit 0
