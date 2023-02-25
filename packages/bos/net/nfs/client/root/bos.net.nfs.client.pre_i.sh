#  @(#)52        1.4  src/packages/bos/net/nfs/client/root/bos.net.nfs.client.pre_i.sh, pkgnfs, pkg411, GOLD410 4/22/94 13:16:33
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
 
#
# NAME:  root/bos.net.nfs.client.pre_i
#
# FUNCTION: script called from instal
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
#   

#
# This deletes the SRC subsystems that were added
# with our config script ( or a previous install)

#============================================================#
#
# Delete the src stuff
#
    rmssys -s rpc.statd >/dev/null 2>&1
    rmssys -s rpc.lockd >/dev/null 2>&1
    rmssys -s rpc.mountd >/dev/null 2>&1
    rmssys -s nfsd >/dev/null 2>&1
    rmssys -s biod >/dev/null 2>&1

    if [ -x /usr/sbin/rmnfs ] ; then
		rmnfs -I >/dev/null 2>&1
    fi

    exit 0
